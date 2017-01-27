#include <Arduino.h>
#include <Pushbutton.h>
#include <AccelStepper.h>
#include <debounce.h>
#include <keymap.h>
#include <model.h>


// define steppers
// todo: enable pins API for power management
AccelStepper carriage(AccelStepper::FULL2WIRE, CARR_00, CARR_01);
AccelStepper daisy(AccelStepper::FULL2WIRE, DASY_00, DASY_01);

// define button inputs
Pushbutton leom_button(LEOM);

void setup() {
  Serial.begin(115200);
  Serial.println("begin");

  // mostly to just keep the board happy
  // floating/input pins will cause board errors
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  // set pins
  pinMode(HAMMER, OUTPUT);
  pinMode(LEOM, INPUT_PULLUP);

  // set HIGH to relax disengage hammer solenoid
  digitalWrite(HAMMER, HIGH);

  // set HIGH to relax the ribbon solenoid
  digitalWrite(RIBBON, HIGH);

  // daisy and linefeed steppers share pins for control
  // different pins will enable/disable the stepper driver

  // LOW will enable daisy stepper
  digitalWrite(DASY_EN, LOW);

  // LOW will enable linefeed stepper
  digitalWrite(LNFD_EN, HIGH);

  // carriage settings
  // maxspeed 500.....480 safe
  // accel 3000 is smooth, but slow
  carriage.setMaxSpeed(450);
  carriage.setAcceleration(3000);

  // linefeed wheel setting
  // max speed 1100.  safe 1000
  // accel 10,000.  seems smooth

  // daisy settings
  // 400 is safe
  daisy.setMaxSpeed(400);
  daisy.setAcceleration(6000);
}

// state machine things.  a hacky prototype for now
#define RESET       0b00000000000000000000000000000000  // 0
#define INIT        0b00000000000000000000000000000001  // 1
#define DASY_HOME   0b00000000000000000000000000000010  // 2
#define DASY_MOVE   0b00000000000000000000000000000100  // 4
#define DASY_ALIGN  0b00000000000000000000000000001000  // 8
#define CARR_HOME   0b00000000000000000000000000010000  // 16
#define CARR_MOVE   0b00000000000000000000000000100000  // 32
#define LEOM_CLOSE  0b00000000000000000000000001000000  // 64
#define CHAR_WAIT   0b00000000000000000000000010000000  // 128
#define HAMMER_RDY  0b00000000000000000000000100000000  // 256
#define MACH_RESET  0b00000000000000000000001000000000  // 512

#define HOMED       DASY_HOME + CARR_HOME

#define DEBUG_PRINT(x) Serial.println(x);
#define CHANGED(x) if (state_changed) {x};
#define STATE_INFO(x) CHANGED(DEBUG_PRINT(x))

// todo: volatile, possibly
bool state_changed = false;
int mach_state = 0;
int old_state = 1;

unsigned long last_punch = 0;
unsigned long punch_interval = 50;  // limit current rush (20 cps)

/* BUGS and other things
   -detect when no wheel is installed
*/

void strike() {
  // todo: non blocking
  digitalWrite(HAMMER, LOW);
  delay(10);  // must have delay, or ribbon will jump!
  digitalWrite(HAMMER, HIGH);
  delay(10);

  // advance ribbon
  digitalWrite(RIBBON, LOW);
  delay(20);
  digitalWrite(RIBBON, HIGH);

  last_punch = millis();
}

void check_carriage() {
  if (!carriage.distanceToGo()) {
    mach_state &= ~CARR_MOVE;
  }
}

void check_daisy() {
  if (!daisy.distanceToGo()) {
    mach_state &= ~DASY_MOVE;
  }
}

void check_leom() {
  if (leom_button.getSingleDebouncedPress()) {
      mach_state |= LEOM_CLOSE;
  } else if (leom_button.getSingleDebouncedRelease()) {
      mach_state &= ~LEOM_CLOSE;
  }
}

void check_hammer() {
    if (last_punch + punch_interval <= millis()) {
      mach_state |= HAMMER_RDY;
    } else {
      mach_state &= ~HAMMER_RDY;
    }
}


void loop() {

  // check for things that will mutate state
  check_carriage();
  check_daisy();
  check_leom();

  // only watch the hammer if it can be unsed
  if (mach_state & HOMED) {
    check_hammer();
  } else {
    mach_state &= ~HAMMER_RDY;
  }

  if (mach_state == old_state) {
    state_changed = false;
  } else {
    state_changed = true;
  }

  switch (mach_state) {
    case RESET:
      // not init, lid closed, no homes
      STATE_INFO("reset")

      // 1. set init

      // parameters for next state
      mach_state |= INIT;
      break;

    case INIT:
      // init, lid closed, no homes
      STATE_INFO("carriage homing...")

      // move left to park the carriage
      carriage.move(9999);  // basically move forever
      mach_state |= CARR_MOVE;

      // if powered off while the daisy wheel is HOME,
      // LEOM will not close when the carriage is parked.
      // rotate the daisy wheel a few turns to offset it
      // and prevent carriage crashes on the left side.
      daisy.move(9999);  // basically move forever
      mach_state |= DASY_MOVE;

      break;

    case INIT + CARR_MOVE + DASY_MOVE + LEOM_CLOSE:
      // wait for home
      STATE_INFO("HOME")

      // set carriage home
      carriage.setCurrentPosition(0);
      mach_state |= CARR_HOME;

      // set diasy wheel home
      daisy.setCurrentPosition(0);
      mach_state |= DASY_HOME;

      break;

    case INIT + HOMED + LEOM_CLOSE + HAMMER_RDY:
      // ready but LEOM disengaged
      STATE_INFO("ready 11")

      carriage.move(-100);
      mach_state |= CARR_MOVE;

      // clear init flag
      mach_state &= ~INIT;

      break;

    case HOMED + HAMMER_RDY:
      strike();

      daisy.move(1);
      mach_state |= DASY_MOVE;

      carriage.move(-5);
      mach_state |= CARR_MOVE;
      break;

    case LEOM_CLOSE:
      // not init, lid open, no homes
      STATE_INFO("lid open")
      // just wait until lid is closed to begin.
      break;

    default:
      STATE_INFO(mach_state)
  }

  // out of sw scope
  old_state = mach_state;
  carriage.run();
  daisy.run();

}
