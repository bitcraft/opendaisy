#ifndef AccelStepper_h
#define AccelStepper_h

#include <stdlib.h>
#include <stdint.h>

#if ARDUINO >= 100
#include <Arduino.h>
#else

#include <WProgram.h>
#include <wiring.h>

#endif

// These defs cause trouble on some versions of Arduino
#undef round

class AccelStepper {
public:

    typedef enum {
        FUNCTION = 0,
        DRIVER = 1,
        FULL2WIRE = 2,
        FULL3WIRE = 3,
        FULL4WIRE = 4,
        HALF3WIRE = 6,
        HALF4WIRE = 8
    } MotorInterfaceType;

    AccelStepper(uint8_t interface = AccelStepper::FULL4WIRE, uint8_t pin1 = 2, uint8_t pin2 = 3, uint8_t pin3 = 4,
                 uint8_t pin4 = 5, bool enable = true);

    AccelStepper(void (*forward)(), void (*backward)());

    float get_speed();

    float get_max_speed();

    bool update();

    bool run_speed();

    bool update_speed();

    long distance_remaining();

    long get_target();

    long get_position();

    void set_speed(float speed);

    void set_backlash_compensation(long amount);

    void set_preferred_direction(bool direction);

    void set_target(long absolute);

    void move_target(long relative);

    void set_home();

    void compute_speed();

    void set_max_speed(float speed);

    void set_acceleration(float acceleration);

    void set_output_pins(uint8_t mask);

    void enable_outputs();

    void disable_outputs();

    void set_min_pulse_width(unsigned int min_width);

    void set_enable_pin(uint8_t enable_pin);

    void set_pins_inverted(bool directionInvert, bool stepInvert, bool enableInvert);

    void set_pins_inverted(bool pin1Invert, bool pin2Invert, bool pin3Invert, bool pin4Invert, bool enableInvert);

    void run();

    bool running();

    void run_to(long position);

    void stop();

    void abort();

protected:

    typedef enum {
        DIRECTION_CCW = 0,
        DIRECTION_CW = 1
    } Direction;

    virtual void step(long step);

    virtual void step0(long step);

    virtual void step1(long step);

    virtual void step2(long step);

    virtual void step3(long step);

    virtual void step4(long step);

    virtual void step6(long step);

    virtual void step8(long step);

private:
    uint8_t _interface;          // 0, 1, 2, 4, 8, See MotorInterfaceType
    uint8_t _pin[4];
    uint8_t _enable_pin;

    bool _pin_inverted[4];
    bool _enable_inverted;

    long _current_pos;    // Steps
    long _target_pos;     // Steps

    float _speed;         // Steps per second
    float _max_speed;
    float _acceleration;

    unsigned long _step_interval;
    unsigned long _last_step_time;
    unsigned int _min_pulse_width;

    void (*_forward)();

    void (*_backward)();

    long _n;
    float _c0;
    float _cn;
    float _cmin; // at max speed
    bool _direction; // 1 == CW

    long _backlash_compensation;
    bool _backlash_correcting;
    bool _preferred_direction;
};

#endif
