// AccelStepper.cpp
//
// Copyright (C) 2009-2013 Mike McCauley
// $Id: AccelStepper.cpp,v 1.23 2016/08/09 00:39:10 mikem Exp $

#include "AccelStepper.h"

#if 0
// Some debugging assistance
void dump(uint8_t* p, int l)
{
    int i;

    for (i = 0; i < l; i++)
    {
    Serial.print(p[i], HEX);
    Serial.print(" ");
    }
    Serial.println("");
}
#endif

AccelStepper::AccelStepper(uint8_t interface, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, bool enable) {
    _interface = interface;
    _pin[0] = pin1;
    _pin[1] = pin2;
    _pin[2] = pin3;
    _pin[3] = pin4;

    _current_pos = 0;
    _target_pos = 0;
    _speed = 0.0;
    _max_speed = 1.0;
    _acceleration = 0.0;
    _step_interval = 0;
    _min_pulse_width = 1;
    _enable_pin = 0xff;
    _last_step_time = 0;

    _backlash_correcting = 0;
    _backlash_compensation = 0;
    _preferred_direction = 0;

    _n = 0;
    _c0 = 0.0;
    _cn = 0.0;
    _cmin = 1.0;
    _direction = DIRECTION_CCW;

    for (int i = 0; i < 4; i++)
        _pin_inverted[i] = 0;

    if (enable)
        enable_outputs();

    // Some reasonable default
    set_acceleration(1);

}

AccelStepper::AccelStepper(void (*forward)(), void (*backward)()) {
    _pin[0] = 0;
    _pin[1] = 0;
    _pin[2] = 0;
    _pin[3] = 0;
    _forward = forward;
    _backward = backward;
    _current_pos = 0;
    _target_pos = 0;
    _speed = 0.0;
    _max_speed = 1.0;
    _acceleration = 0.0;
    _step_interval = 0;
    _min_pulse_width = 1;
    _enable_pin = 0xff;
    _last_step_time = 0;

    _backlash_correcting = 0;
    _backlash_compensation = 0;
    _preferred_direction = 0;

    _n = 0;
    _c0 = 0.0;
    _cn = 0.0;
    _cmin = 1.0;
    _direction = DIRECTION_CCW;

    for (int i = 0; i < 4; i++)
        _pin_inverted[i] = 0;

    // Some reasonable default
    set_acceleration(1);
}

void AccelStepper::set_backlash_compensation(long amount) {
    _backlash_compensation = amount;
}

void AccelStepper::set_preferred_direction(bool direction) {
    _preferred_direction = direction;
}

void AccelStepper::set_target(long absolute) {
    long distance = absolute - _current_pos;
    bool direction = (distance > 0) ? DIRECTION_CW : DIRECTION_CCW;

    // is this target different from the current one?
    if (_target_pos != absolute) {

        // is the direction of movement preferred or not?
        if (direction != _preferred_direction) {

            // record that we need to compensation for backlash at the end of this movement
            _backlash_correcting = 1;

            // TODO: check last direction moved

            if (direction == DIRECTION_CW) {
                absolute += _backlash_compensation;  // ok!
            } else {
                absolute -= _backlash_compensation;
            }
        }

        // record new target position
        _target_pos = absolute;

        // schedule next step
        compute_speed();

        // TODO: compute new n?
    }
}

void AccelStepper::move_target(long relative) {
    set_target(_current_pos + relative);
}

// Run the motor to implement speed and acceleration in order to proceed to the target position
// You must call this at least once per step, preferably in your main loop
// If the motor is in the desired position, the cost is very small
// returns true if the motor is still running to the target position.
bool AccelStepper::update() {
    if (update_speed())
        compute_speed();
    return _speed != 0.0 || distance_remaining() != 0 || _backlash_correcting;
}

// Implements steps according to the current step interval
// You must call this at least once per step
// returns true if a step occurred
bool AccelStepper::update_speed() {
    // Don't do anything unless we actually have a step interval
    if (!_step_interval)
        return false;

    unsigned long time = micros();
    if (time - _last_step_time >= _step_interval) {
        if (_direction == DIRECTION_CW) {
            _current_pos += 1;
        } else {
            // Anticlockwise
            _current_pos -= 1;
        }
        step(_current_pos);

        _last_step_time = time; // Caution: does not account for costs in step()

        return true;
    } else {
        return false;
    }
}

long AccelStepper::distance_remaining() {
    return _target_pos - _current_pos;
}

long AccelStepper::get_target() {
    return _target_pos;
}

long AccelStepper::get_position() {
    return _current_pos;
}

// Useful during initialisations or after initial positioning
// Sets speed to 0
void AccelStepper::set_home() {
    _target_pos = _current_pos = 0;
    _n = 0;
    _step_interval = 0;
    _speed = 0.0;
    _backlash_correcting = 0;
    // TODO: check last direction moved
}

void AccelStepper::compute_speed() {
    long distance = distance_remaining(); // +ve is clockwise from current location
    long steps_remaining = (long) ((_speed * _speed) / (2.0 * _acceleration)); // Equation 16

    // We are at the target and its time to stop, maybe
    if (distance == 0 && steps_remaining <= 1) {

        // backlash compensation is enabled, and we are at the stop point
        if (_backlash_compensation && _backlash_correcting) {
            _step_interval = 0;
            _speed = 0.0;
            _n = 0;

            // at this point, direction will be reversed to reduce
            // backlash.  _direction will be the direction it was
            // previously traveling.
            // bl dir is 0, direction is ?  then use -
            // ok if p_dir is 0
            if (_direction == DIRECTION_CW) {
                move_target(-_backlash_compensation);  // ok!
            } else {
                move_target(_backlash_compensation);
            }
            _backlash_correcting = 0;

            // re-evaluate the steps and distance
            distance = distance_remaining(); // +ve is clockwise from current location
            steps_remaining = (long) ((_speed * _speed) / (2.0 * _acceleration)); // Equation 16

        } else {
            // set these parameters to stop completely
            _backlash_correcting = 0;
            _step_interval = 0;
            _speed = 0.0;
            _n = 0;
            return;
        }
    }

    if (distance > 0) {
        // We are anticlockwise from the target
        // Need to go clockwise from here, maybe decelerate now
        if (_n > 0) {
            // Currently accelerating, need to decel now? Or maybe going the wrong way?
            if ((steps_remaining >= distance) || _direction == DIRECTION_CCW)
                _n = -steps_remaining; // Start deceleration
        } else if (_n < 0) {
            // Currently decelerating, need to accel again?
            if ((steps_remaining < distance) && _direction == DIRECTION_CW)
                _n = -_n; // Start acceleration
        }
    } else if (distance < 0) {
        // We are clockwise from the target
        // Need to go anticlockwise from here, maybe decelerate
        if (_n > 0) {
            // Currently accelerating, need to decel now? Or maybe going the wrong way?
            if ((steps_remaining >= -distance) || _direction == DIRECTION_CW)
                _n = -steps_remaining; // Start deceleration
        } else if (_n < 0) {
            // Currently decelerating, need to accel again?
            if ((steps_remaining < -distance) && _direction == DIRECTION_CCW)
                _n = -_n; // Start acceleration
        }
    }

    // Need to accelerate or decelerate
    if (_n == 0) {
        // First step from stopped
        _cn = _c0;
        _direction = (distance > 0) ? DIRECTION_CW : DIRECTION_CCW;
    } else {
        // Subsequent step. Works for accel (n is +_ve) and decel (n is -ve).
        _cn = _cn - ((2.0 * _cn) / ((4.0 * _n) + 1)); // Equation 13
        _cn = max(_cn, _cmin);
    }
    _n++;
    _step_interval = _cn;
    _speed = 1000000.0 / _cn;
    if (_direction == DIRECTION_CCW)
        _speed = -_speed;

    // Serial.println(_direction);
    // Serial.println(_speed);
    // Serial.println(_acceleration);
    // Serial.println(_cn);
    // Serial.println(_c0);
    // Serial.println(_n);
    // Serial.println(_step_interval);
    // Serial.println(distance);
    // Serial.println(steps_remaining);
    // Serial.println(_backlash_compensation);
    // Serial.println(_backlash_correcting);
    // Serial.println(_preferred_direction);
    // Serial.println("-----");

}

float AccelStepper::get_max_speed() {
    return _max_speed;
}

void AccelStepper::set_max_speed(float speed) {
    if (_max_speed != speed) {
        _max_speed = speed;
        _cmin = 1000000.0 / speed;
        // Recompute _n from current speed and adjust speed if accelerating or cruising
        if (_n > 0) {
            _n = (long) ((_speed * _speed) / (2.0 * _acceleration)); // Equation 16
            compute_speed();
        }
    }
}

void AccelStepper::set_acceleration(float acceleration) {
    if (acceleration == 0.0)
        return;
    if (_acceleration != acceleration) {
        // Recompute _n per Equation 17
        _n = _n * (_acceleration / acceleration);
        // New c0 per Equation 7, with correction per Equation 15
        _c0 = 0.676 * sqrt(2.0 / acceleration) * 1000000.0; // Equation 15
        _acceleration = acceleration;
        compute_speed();
    }
}

float AccelStepper::get_speed() {
    return _speed;
}

void AccelStepper::set_speed(float speed) {
    // don't do anything if the speed is not different
    if (speed == _speed)
        return;

    speed = constrain(speed, -_max_speed, _max_speed);

    if (speed == 0.0)
        _step_interval = 0;
    else {
        _step_interval = fabs(1000000.0 / speed);
        _direction = (speed > 0.0) ? DIRECTION_CW : DIRECTION_CCW;
    }

    _speed = speed;
}

// You might want to override this to implement eg serial output
// bit 0 of the mask corresponds to _pin[0]
// bit 1 of the mask corresponds to _pin[1]
// ....
void AccelStepper::set_output_pins(uint8_t mask) {
    uint8_t num_pins = 2;
    if (_interface == FULL4WIRE || _interface == HALF4WIRE)
        num_pins = 4;
    else if (_interface == FULL3WIRE || _interface == HALF3WIRE)
        num_pins = 3;
    uint8_t i;
    for (i = 0; i < num_pins; i++)
        digitalWrite(_pin[i], (mask & (1 << i)) ? (HIGH ^ _pin_inverted[i]) : (LOW ^ _pin_inverted[i]));
}

// Subclasses can override
void AccelStepper::step(long step) {
    switch (_interface) {
        case FUNCTION:
            step0(step);
            break;

        case DRIVER:
            step1(step);
            break;

        case FULL2WIRE:
            step2(step);
            break;

        case FULL3WIRE:
            step3(step);
            break;

        case FULL4WIRE:
            step4(step);
            break;

        case HALF3WIRE:
            step6(step);
            break;

        case HALF4WIRE:
            step8(step);
            break;
    }
}

// 0 pin step function (ie for functional usage)
void AccelStepper::step0(long step) {
    (void) (step); // Unused
    if (_speed > 0)
        _forward();
    else
        _backward();
}

// 1 pin step function (ie for stepper drivers)
// This is passed the current step number (0 to 7)
// Subclasses can override
void AccelStepper::step1(long step) {
    (void) (step); // Unused

    // _pin[0] is step, _pin[1] is direction
    set_output_pins(_direction ? 0b10 : 0b00); // Set direction first else get rogue pulses
    set_output_pins(_direction ? 0b11 : 0b01); // step HIGH
    // Caution 200ns setup time
    // Delay the minimum allowed pulse width
    delayMicroseconds(_min_pulse_width);
    set_output_pins(_direction ? 0b10 : 0b00); // step LOW
}

// 2 pin step function
// This is passed the current step number (0 to 7)
// Subclasses can override
void AccelStepper::step2(long step) {
    switch (step & 0x3) {
        case 0: /* 01 */
            set_output_pins(0b10);
            break;

        case 1: /* 11 */
            set_output_pins(0b11);
            break;

        case 2: /* 10 */
            set_output_pins(0b01);
            break;

        case 3: /* 00 */
            set_output_pins(0b00);
            break;
    }
}

// 3 pin step function
// This is passed the current step number (0 to 7)
// Subclasses can override
void AccelStepper::step3(long step) {
    switch (step % 3) {
        case 0:    // 100
            set_output_pins(0b100);
            break;

        case 1:    // 001
            set_output_pins(0b001);
            break;

        case 2:    //010
            set_output_pins(0b010);
            break;

    }
}

// 4 pin step function for half stepper
// This is passed the current step number (0 to 7)
// Subclasses can override
void AccelStepper::step4(long step) {
    switch (step & 0x3) {
        case 0:    // 1010
            set_output_pins(0b0101);
            break;

        case 1:    // 0110
            set_output_pins(0b0110);
            break;

        case 2:    //0101
            set_output_pins(0b1010);
            break;

        case 3:    //1001
            set_output_pins(0b1001);
            break;
    }
}

// 3 pin half step function
// This is passed the current step number (0 to 7)
// Subclasses can override
void AccelStepper::step6(long step) {
    switch (step % 6) {
        case 0:    // 100
            set_output_pins(0b100);
            break;

        case 1:    // 101
            set_output_pins(0b101);
            break;

        case 2:    // 001
            set_output_pins(0b001);
            break;

        case 3:    // 011
            set_output_pins(0b011);
            break;

        case 4:    // 010
            set_output_pins(0b010);
            break;

        case 5:    // 011
            set_output_pins(0b110);
            break;

    }
}

// 4 pin half step function
// This is passed the current step number (0 to 7)
// Subclasses can override
void AccelStepper::step8(long step) {
    switch (step & 0x7) {
        case 0:    // 1000
            set_output_pins(0b0001);
            break;

        case 1:    // 1010
            set_output_pins(0b0101);
            break;

        case 2:    // 0010
            set_output_pins(0b0100);
            break;

        case 3:    // 0110
            set_output_pins(0b0110);
            break;

        case 4:    // 0100
            set_output_pins(0b0010);
            break;

        case 5:    //0101
            set_output_pins(0b1010);
            break;

        case 6:    // 0001
            set_output_pins(0b1000);
            break;

        case 7:    //1001
            set_output_pins(0b1001);
            break;
    }
}

void AccelStepper::enable_outputs() {
    if (!_interface)
        return;

    pinMode(_pin[0], OUTPUT);
    pinMode(_pin[1], OUTPUT);
    if (_interface == FULL4WIRE || _interface == HALF4WIRE) {
        pinMode(_pin[2], OUTPUT);
        pinMode(_pin[3], OUTPUT);
    } else if (_interface == FULL3WIRE || _interface == HALF3WIRE) {
        pinMode(_pin[2], OUTPUT);
    }

    if (_enable_pin != 0xff) {
        pinMode(_enable_pin, OUTPUT);
        digitalWrite(_enable_pin, HIGH ^ _enable_inverted);
    }
}

// Prevents power consumption on the outputs
void AccelStepper::disable_outputs() {
    if (!_interface) return;

    set_output_pins(0); // Handles inversion automatically
    if (_enable_pin != 0xff) {
        pinMode(_enable_pin, OUTPUT);
        digitalWrite(_enable_pin, LOW ^ _enable_inverted);
    }
}

void AccelStepper::set_min_pulse_width(unsigned int min_width) {
    _min_pulse_width = min_width;
}

void AccelStepper::set_enable_pin(uint8_t enable_pin) {
    _enable_pin = enable_pin;

    // This happens after construction, so init pin now.
    if (_enable_pin != 0xff) {
        pinMode(_enable_pin, OUTPUT);
        digitalWrite(_enable_pin, HIGH ^ _enable_inverted);
    }
}

void AccelStepper::set_pins_inverted(bool directionInvert, bool stepInvert, bool enableInvert) {
    _pin_inverted[0] = stepInvert;
    _pin_inverted[1] = directionInvert;
    _enable_inverted = enableInvert;
}

void
AccelStepper::set_pins_inverted(bool pin1Invert, bool pin2Invert, bool pin3Invert, bool pin4Invert, bool enableInvert) {
    _pin_inverted[0] = pin1Invert;
    _pin_inverted[1] = pin2Invert;
    _pin_inverted[2] = pin3Invert;
    _pin_inverted[3] = pin4Invert;
    _enable_inverted = enableInvert;
}

// Blocks until the target position is reached and stopped
void AccelStepper::run() {
    while (update());
}

bool AccelStepper::run_speed() {
    if (_target_pos == _current_pos)
        return false;
    if (_target_pos > _current_pos)
        _direction = DIRECTION_CW;
    else
        _direction = DIRECTION_CCW;
    return update_speed();
}

// Blocks until the new target position is reached
void AccelStepper::run_to(long position) {
    set_target(position);
    run();
}

void AccelStepper::stop() {
    if (_speed != 0.0) {
        long steps_remaining = (long) ((_speed * _speed) / (2.0 * _acceleration)) + 1; // Equation 16 (+integer rounding)
        if (_speed > 0)
            move_target(steps_remaining);
        else
            move_target(-steps_remaining);
    }
}

void AccelStepper::abort() {
  _step_interval = 0;
  _backlash_correcting = 0;
  _target_pos = _current_pos;
  _speed = 0;
  _n = 0;
}

bool AccelStepper::running() {
    return !(_speed == 0.0 && _target_pos == _current_pos && !_backlash_correcting);
}
