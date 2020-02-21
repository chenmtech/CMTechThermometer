/*
 * The thermometer application function header file
 * This model is used to mainly realize all the functions related to measure the thermometer
*/

#ifndef APP_THERMOMETER_H
#define APP_THERMOMETER_H

#include "comdef.h"


// measurement controll flag, Three types of the measurement output value
#define THERMOMETER_CFG_VALUETYPE_AD    0x01    //AD value
#define THERMOMETER_CFG_VALUETYPE_R     0x02    //Resistor value
#define THERMOMETER_CFG_VALUETYPE_T     0x03    //Temperature value

// Temperature limit value
#define LOWLIMIT_T    3290
#define UPLIMIT_T     4410

// Resistor limit value
#define LOWLIMIT_R    22332
#define UPLIMIT_R     34950

// AD limit value
#define LOWLIMIT_AD   0
#define UPLIMIT_AD    32768

// initialize the model
extern void Thermo_Init();

// turn off the hardware, including saving the max value and turning off the LCD and ADC
extern void Thermo_HardwareOff();

// turn on the hardware, including reading and showing the last max value and turnning on the LCD and ADC
extern void Thermo_HardwareOn();

// get the value which can be AD, R or T value according to the value type
extern uint16 Thermo_GetValue();

// do calibration experiment for a new device in order to get its "caliValue"
extern void Thermo_DoCaliExperiment();

// update max value
extern uint16 Thermo_UpdateMaxValue(uint16 value);

// show a value on the LDC
extern void Thermo_ShowValueOnLCD(uint8 location, uint16 value);

#endif