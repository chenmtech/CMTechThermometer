/*
 * The thermometer application model header file
 * This model is used to mainly realize all the functions related to measure the thermometer
*/

#ifndef APP_THERMOMETER_H
#define APP_THERMOMETER_H

#include "comdef.h"


// measurement controll flag
#define THERMOMETER_CFG_STANDBY         0x00    //stop measurement into standby mode
// Three types of the measurement output value
#define THERMOMETER_CFG_VALUETYPE_AD    0x01    //AD value
#define THERMOMETER_CFG_VALUETYPE_R     0x02    //Resistor value
#define THERMOMETER_CFG_VALUETYPE_T     0x03    //Temperature value
#define THERMOMETER_CFG_CALIBRATION     0x04    // do calibration 
#define THERMOMETER_CFG_LCDON           0x05    // turn on the LCD
#define THERMOMETER_CFG_LCDOFF          0x06    // turn off the LCD

#define LOWLIMIT_T    3290    // low limit of the Temperature value
#define UPLIMIT_T     4410    // up limit of the Temperature value

#define LOWLIMIT_R    22332    // low limit of the Resistor value
#define UPLIMIT_R     34950    // up limit of the Resistor value

#define LOWLIMIT_AD   0        // low limit of the AD value
#define UPLIMIT_AD    32768    // up limit of the AD value


// initialize the model
extern void Thermo_Init();

// save the max value and then turn off the hardware, including the LCD and ADC
extern void Thermo_HardwareOff();

// turn on the hardware, including the LCD and ADC, and then read the last max value and show it.
extern void Thermo_HardwareOn();

// get value type
extern uint8 Thermo_GetValueType();

// set value type
extern void Thermo_SetValueType(uint8 type);

// get value which can be AD, R or T value according to the value type
extern uint16 Thermo_GetValue();

// do calibration for a new device in order to get its caliValue
extern void Thermo_DoCalibration();

// 更新当前最大值
extern uint16 Thermo_UpdateMaxValue(uint16 value);

// 在LCD上显示一个值
extern void Thermo_ShowValueOnLCD(uint8 location, uint16 value);

// 开LCD
extern void Thermo_LCDOn();

// 关LCD
extern void Thermo_LCDOff();

// 开蜂鸣器
extern void Thermo_ToneOn();

// 关蜂鸣器
extern void Thermo_ToneOff();

#endif