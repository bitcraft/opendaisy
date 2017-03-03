// pin mappings for typewriter board

// some model specific things

#define PIN_RIBBON_BLK 0x00
#define PIN_RIBBON_WHT 0x01

// following constants define pins

//carriage
#define PIN_CARR_00 0x03  // todo: swap?  seems to be miswired
#define PIN_CARR_01 0x02  // todo: ^^^
#define PIN_CARR_02 0x00
#define PIN_CARR_03 0x00
#define CARR_FINE   30
#define CARR_SLOW   80
#define CARR_FAST   400
#define CARR_ACCEL  3000

//daisy
#define PIN_DASY_00 0x04
#define PIN_DASY_01 0x05
#define PIN_DASY_02 0x00
#define PIN_DASY_03 0x00
#define PIN_DASY_EN 0x07
#define DASY_SLOW   200
#define DASY_FAST   700
#define DASY_LEN    96
#define DASY_ACCEL  7000

//linefeed
#define PIN_LNFD_00 0x04
#define PIN_LNFD_01 0x05
#define PIN_LNFD_02 0x00
#define PIN_LNFD_03 0x00
#define PIN_LNFD_EN 0x06

// cover open sensor
#define PIN_COVR_00 0x00

// keyboard matrix
#define PIN_KEYM_00 0x00
#define PIN_KEYM_01 0x00
#define PIN_KEYM_02 0x00
#define PIN_KEYM_03 0x00
#define PIN_KEYM_04 0x00
#define PIN_KEYM_05 0x00
#define PIN_KEYM_06 0x00
#define PIN_KEYM_07 0x00

// speaker
#define PIN_SPKR_00 0x00

// lights
#define PIN_LED_00 0x00

// hammer
#define PIN_HAMMER 0x08

// ribbon
#define PIN_RIBBON 0x09

// LEOM
#define PIN_LEOM 0x0C
