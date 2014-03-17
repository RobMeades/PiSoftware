/*
 * One Wire bus HW configurations - what the pins do.
 */ 
 
/*
 * MANIFEST CONSTANTS
 */

/*
 * These are the masks for how the various IO ports are
 * connected.  To set an IO port to 5V (==high impedance
 * with pull-up resistor), make it a 1, to set it to ground
 * (== the transistor is on, i.e. low impedance) make it a 0.
 */

/* How the pins are connected on the "charger state" PIO chip */
#define CHARGER_STATE_RIO_GREEN  0x01
#define CHARGER_STATE_RIO_RED    0x02
#define CHARGER_STATE_O1_GREEN   0x04
#define CHARGER_STATE_O1_RED     0x08
#define CHARGER_STATE_O2_GREEN   0x10
#define CHARGER_STATE_O2_RED     0x20
#define CHARGER_STATE_O3_GREEN   0x40
#define CHARGER_STATE_O3_RED     0x80

/* How the pins are connected on the "Schottky" PIO chip */
#define SCHOTTKY_NON_SCHOTTKY_SPARE_1  0x01/* Note that this is NOT via the Schottky chip, it is simply a spare IO, brought out on pin 1 of the spare IO header */
#define SCHOTTKY_RIO_PWR_12V_ON        0x02
#define SCHOTTKY_RIO_PWR_BATT_OFF      0x04
#define SCHOTTKY_O_PWR_TOGGLE          0x08
#define SCHOTTKY_O_RESET_TOGGLE        0x10
#define SCHOTTKY_SPARE_3               0x20 /* Connected to pin 3 of the spare header */
#define SCHOTTKY_SPARE_4               0x40 /* Connected to pin 4 of the spare header */
#define SCHOTTKY_ENABLE                0x80 /* Note that this is NOT via the Schottky chip, it is used as an enable TO the Schottky chip */

/* How the pins are connected on the "Relay" PIO chip */
#define RELAY_O_PWR_12V_ON           0x01
#define RELAY_O_PWR_BATT_OFF         0x02
#define RELAY_RIO_CHARGER_ON         0x04
#define RELAY_O1_CHARGER_ON          0x08
#define RELAY_O2_CHARGER_ON          0x10
#define RELAY_O3_CHARGER_ON          0x20
#define RELAY_ENABLE                 0x40
#define RELAY_SPARE_2                0x80 /* Connected to pin 2 of the spare IO header */

/* How the pins are connected on the "General Purpose" PIO chip */
#define GENERAL_PURPOSE_IO_0         0x01
#define GENERAL_PURPOSE_IO_1         0x02
#define GENERAL_PURPOSE_IO_2         0x04
#define GENERAL_PURPOSE_IO_3         0x08
#define GENERAL_PURPOSE_IO_4         0x10
#define GENERAL_PURPOSE_IO_5         0x20
#define GENERAL_PURPOSE_IO_6         0x40
#define GENERAL_PURPOSE_IO_7         0x80

#define GENERAL_PURPOSE_IO_MUX_A0    GENERAL_PURPOSE_IO_0 /* If J1 is connected, General Purpose IO 0 is connected to A0 of the mux chip */
#define GENERAL_PURPOSE_IO_MUX_A1    GENERAL_PURPOSE_IO_1 /* If J2 is connected, General Purpose IO 1 is connected to A1 of the mux chip */
#define GENERAL_PURPOSE_IO_MUX_A2    GENERAL_PURPOSE_IO_2 /* If J3 is connected, General Purpose IO 2 is connected to A2 of the mux chip */
#define GENERAL_PURPOSE_IO_MUX_E     GENERAL_PURPOSE_IO_3 /* If J4 is connected, General Purpose IO 3 is connected to ~E of the mux chip */
