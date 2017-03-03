#include <Arduino.h>
#include <Pushbutton.h>
#include <AccelStepper.h>
#include <debounce.h>
#include <keymap.h>
#include <model.h>

const boolean DIRECTION_CCW = 0;  ///< Clockwise
const boolean DIRECTION_CW = 1;   ///< Counter-Clockwise


// define carriage
AccelStepper carriage(AccelStepper::FULL2WIRE, PIN_CARR_00, PIN_CARR_01);

// define daisy wheel
AccelStepper daisy(AccelStepper::FULL2WIRE, PIN_DASY_00, PIN_DASY_01);

// define line feed
AccelStepper linefeed(AccelStepper::FULL2WIRE, PIN_LNFD_00, PIN_LNFD_02);

// define button inputs
Pushbutton leom_button(PIN_LEOM);


void setup() {
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

  // debugging
  Serial.begin(115200);
  Serial.println("begin");

  // set pins
  pinMode(PIN_HAMMER, OUTPUT);
  pinMode(PIN_LEOM, INPUT_PULLUP);

  // set HIGH to relax the hammer solenoid
  // setting LOW will engage the solenoid and hold it
  digitalWrite(PIN_HAMMER, HIGH);

  // set HIGH to relax the ribbon solenoid
  // setting LOW will engage the solenoid and hold it
  digitalWrite(PIN_RIBBON, HIGH);

  // daisy and linefeed steppers share pins for control
  // different pins will enable/disable the stepper driver

  // LOW will enable daisy stepper
  digitalWrite(PIN_DASY_EN, LOW);

  // LOW will enable linefeed stepper
  digitalWrite(PIN_LNFD_EN, HIGH);

  // linefeed wheel setting
  // max speed 1100.  safe 1000
	// accel 10,000.  seems smooth
  // carriage.set_preferred_direction(DIRECTION_CCW);
  // carriage.set_backlash_compensation(10);

  // CW is targets +1, CCW is -1

  daisy.set_preferred_direction(DIRECTION_CW);
  daisy.set_backlash_compensation(1);
  daisy.set_enable_pin(PIN_DASY_EN);
  daisy.set_pins_inverted(1, 1, 1);

  advance_ribbon();
}

// state machine things.  a hacky prototype for now
#define RESET         0b00000000000000000000000000000000  // 0
#define INIT          0b00000000000000000000000000000001  // 1
#define ESCAPE        0b00000000000000000000000000000010  // 2
#define HOMING1       0b00000000000000000000000000000100  // 4
#define HOMING2       0b00000000000000000000000000001000  // 8
#define HOMING3       0b00000000000000000000000000010000  // 16
#define HOMING4       0b00000000000000000000000000100000  // 32
#define NORMAL        0b00000000000000000000000001000000  // 64
#define LEOM          0b00000000000000000000000010000000  // 128
#define DASY_MOVING   0b00000000000000000000000100000000  // 256
#define CARR_MOVING   0b00000000000000000000001000000000  // 512
#define HAMMER_RDY    0b00000000000000000000010000000000  // 1024
#define MARGIN_LEFT   0b00000000000000000000100000000000  // 2048
#define MARGIN_RIGHT  0b00000000000000000001000000000000  // 4096
#define CHAR_WAITING  0b00000000000000000010000000000000  // 81926

#define IDLE HAMMER_RDY

#define DEBUG_PRINT(x) Serial.println(x);
#define CHANGED(x) if (state_changed) {x};
#define STATE_INFO(x) CHANGED(DEBUG_PRINT(x))

// todo: volatile, possibly
bool state_changed = false;
int mach_state = 0;
int old_state = 1;

unsigned long last_punch = 0;
unsigned long punch_interval = 20;  // limit current rush (20 cps)

/* BUGS and other things
   -detect when no wheel is installed
*/

void strike() {
  // todo: non blocking
  // hammer

  //digitalWrite(PIN_HAMMER, LOW);
  analogWrite(PIN_HAMMER, 0);
  delay(1);  // must have delay, or ribbon will jump!
  analogWrite(PIN_HAMMER, 12);  // 24 is light, 8 is dark, 0 too hard
  delay(5);  // must have delay, or ribbon will jump!
  digitalWrite(PIN_HAMMER, HIGH);
  delay(5);

  advance_ribbon();

  last_punch = millis();
}

void advance_ribbon() {

  // advance ribbon
  //digitalWrite(PIN_RIBBON, LOW);
  digitalWrite(PIN_RIBBON, LOW);
  delay(4);
  digitalWrite(PIN_RIBBON, HIGH);
  delay(4);
  digitalWrite(PIN_RIBBON, LOW);
  delay(4);
  digitalWrite(PIN_RIBBON, HIGH);

}

void check_carriage() {
  if (carriage.running()) {
    mach_state |= CARR_MOVING;
  } else {
    mach_state &= ~CARR_MOVING;
  }
}

void check_daisy() {
  if (daisy.running()) {
    mach_state |= DASY_MOVING;
  } else {
    mach_state &= ~DASY_MOVING;
  }
}

void check_leom() {
  if (leom_button.getSingleDebouncedPress()) {
      mach_state |= LEOM;
  } else if (leom_button.getSingleDebouncedRelease()) {
      mach_state &= ~LEOM;
  }
}

void check_hammer() {
    if (last_punch + punch_interval <= millis()) {
      mach_state |= HAMMER_RDY;
    } else {
      mach_state &= ~HAMMER_RDY;
    }
}

// i really don't know how to correctly handle unicode
// but this works, somehow
String test_string = "All work and no play makes Jack a dull boy.";
char single_char;
unsigned int temp_int;
int char_index = 0;
int wheel_index = 0;

bool reset_wheel = false;

int find_index(int value) {
    for (int i=0; i<96; i++) {
	     if (ascii[i] == value) {
         return(i);  /* it was found */
	    }
   }
   return(-1);  /* if it was not found */
}

void loop() {

  // check for things that will mutate state
  check_carriage();
  check_daisy();
  check_leom();
  check_hammer();

  // give steppers time to move
  carriage.update();
  daisy.update();
	linefeed.update();

	// detect if there was a state change
  state_changed = mach_state != old_state;

  switch (mach_state) {

    // reset
    case 0:
      // not init, lid closed, no homes
      STATE_INFO("reset")

      // needed?
      carriage.set_home();
      daisy.set_home();

      // transition to INIT
      mach_state |= ESCAPE;

      break;

    // ESCAPE
    // move right some in case was parked when powered off
    case ESCAPE + IDLE:
      STATE_INFO("ESCAPE")

      // slowly move left to check if carriage is parked
      carriage.set_max_speed(CARR_SLOW);
      carriage.set_acceleration(CARR_ACCEL);
      carriage.move_target(30);
      carriage.enable_outputs();

      // spin several times to lock the wheel
      // user may have placed in a new one
      daisy.set_max_speed(DASY_FAST);
      daisy.set_acceleration(DASY_ACCEL);
      daisy.set_target(300);  // 4 rotations
      daisy.enable_outputs();

      mach_state &= ~ESCAPE;
      mach_state |= HOMING1;

      break;

    // INIT + HAMMER_RDY
    case HOMING1 + IDLE:
      // init, lid closed, no homes
      STATE_INFO("begin init, homing...")
      // move left to park the carriage, and spin wheel
      // set carriage to slow speed for fine control
      carriage.set_max_speed(CARR_SLOW);
      carriage.set_acceleration(CARR_ACCEL);
      carriage.set_target(-9999);  // basically move for a long time
      carriage.enable_outputs();

      // negative or positive direction affects the margins
      // - will tend to choose ' or d as first
      // + will choose g

      // set daisy to fast speed to reset the typewheel
      // and to catch the LEOM switch more quickly
      daisy.set_max_speed(DASY_SLOW);
      daisy.set_acceleration(DASY_ACCEL);
      daisy.set_target(9999);
      daisy.enable_outputs();

      // set the wheel in case it was changed while off
      //daisy.move(DASY_LEN * 2);

      // begin homing of carriage and wheel
      mach_state &= ~INIT;

      break;

      // DASY_MOVING + DASY_HOMING + CARR_MOVING + CARR_HOMING + HAMMER_RDY
      case HOMING1 + IDLE + LEOM + DASY_MOVING + CARR_MOVING:
        STATE_INFO("COARSE HOME FOUND")

        // found the home, but it may be slightly offset
        // back out to clear LEOM, and move wheel to make
        // sure it will engage LEOM again

        // move right some
        carriage.abort();
        carriage.set_max_speed(CARR_FAST);
        carriage.move_target(30);
        carriage.enable_outputs();

    		// offset the wheel, so that LEOM will trip reliably
        daisy.abort();
        daisy.move_target(24);
        daisy.enable_outputs();

        mach_state &= ~HOMING1;
        mach_state |= HOMING2;

        break;

    // DASY_HOMING + CARR_HOMING + HAMMER_RDY + REHOME:
    case HOMING2 + IDLE:
      STATE_INFO("REHOME")

      // reached point outside the parking position
      // move back into it again
      carriage.set_max_speed(CARR_FINE);
      carriage.move_target(-40);
      carriage.enable_outputs();

      mach_state &= ~HOMING2;
      mach_state |= HOMING3;

      break;

    case HOMING3 + LEOM + CARR_MOVING + IDLE:
      STATE_INFO("CARR HOMED")

      // move left a little to ensure LEOM is triggered
      carriage.abort();
      carriage.move_target(-14);
      carriage.enable_outputs();

      mach_state &= ~HOMING3;
      mach_state |= HOMING4;

      break;

    // parked, carriage ready, now find wheel home
    // most reliable way is to knock the LEOM latch  :(
    case HOMING4 + LEOM + IDLE:
      STATE_INFO("WHELL HOMING")
      daisy.abort();
      daisy.move_target(140);
      daisy.enable_outputs();
      break;

    case HOMING4 + IDLE:
      STATE_INFO("HOMED ALL")

      carriage.set_home();
      carriage.set_max_speed(CARR_FAST);

      daisy.set_home();
      daisy.set_max_speed(DASY_FAST);

      delay(100);

      // transition to NORMAL
      mach_state |= NORMAL;
      mach_state |= MARGIN_LEFT;
      mach_state &= ~HOMING4;

      break;

    case NORMAL + IDLE + MARGIN_LEFT:
      // move to left MARGIN_LEFT
      STATE_INFO("LEFT MARGIN")

      // move to the left margin
      carriage.set_max_speed(CARR_FAST);
      carriage.set_target(50);
      carriage.enable_outputs();

      mach_state &= ~MARGIN_LEFT;
      break;

    case NORMAL + HAMMER_RDY:
      STATE_INFO("NEXT CHAR")
      Serial.println(char_index);

      // get char from the input string
      single_char = test_string[char_index];

      // advance buffer index
      Serial.println(single_char);
      char_index += 1;

      // handle space
      if (single_char == 32) {
        // do nothing
        carriage.move_target(8);
        carriage.enable_outputs();

      } else {
        // translate the char to our charset
        temp_int = find_index(single_char);

        // move the daisy wheel the the correct position
        Serial.println(temp_int);
        daisy.set_target(temp_int);
        daisy.enable_outputs();

        mach_state |= CHAR_WAITING;
      }

      break;

    case NORMAL + HAMMER_RDY + CHAR_WAITING:
      STATE_INFO("STRIKE")

      delay(30);
      strike();
      delay(100);

      // TODO: add small gap between hammer stike and carriage move

      carriage.move_target(8);

      mach_state &= ~CHAR_WAITING;
      break;

    // case INIT + HOMED + LEOM + HAMMER_RDY:
    //   // ready but LEOM disengaged
    //   STATE_INFO("ready 11")
    //
    //   carriage.move(-100);
    //
    //
    //   break;
    //
    // case HOMED + HAMMER_RDY:
    //   strike();
    //
    //   daisy.move(1);
    //   carriage.move(-5);
    //
    //   break;

    // leom
    case 8:
      // not init, lid open, no homes
      STATE_INFO("lid open")
      // just wait until lid is closed to begin.
      break;

    // HOMING + DASY_MOVING + DASY_HOMING + CARR_MOVING
    case 434231:
      STATE_INFO("carriage")
      break;

    // DASY_MOVING + CARR_MOVING + CARR_HOMING + HAMMER_RDY
    case 3423412:
      STATE_INFO("carriage?")

    default:
      STATE_INFO(mach_state)
  }

  // out of sw scope
  old_state = mach_state;
}
