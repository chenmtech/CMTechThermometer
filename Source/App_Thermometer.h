/*
 * The thermometer application model header file
 * This model is used to mainly execute measuring the thermometer
*/

#ifndef APP_THERMOMETER_H
#define APP_THERMOMETER_H

#include "comdef.h"

// Three types of the measurement output value
#define THERMOMETER_VALUETYPE_AD    0x01    //AD value
#define THERMOMETER_VALUETYPE_R     0x02    //Resistor value
#define THERMOMETER_VALUETYPE_T     0x03    //Temperature value

#define T_LOWLIMIT    3390    // low limit of the Temperature value
#define T_UPLIMIT     4410    // up limit of the Temperature value

#define R_LOWLIMIT    22332    // low limit of the Resistor value
#define R_UPLIMIT     33528    // up limit of the Resistor value

#define AD_LOWLIMIT   0        // low limit of the AD value
#define AD_UPLIMIT    32768    // up limit of the AD value


// 初始化
extern void Thermo_Init();

// 关硬件
extern void Thermo_HardwareOff();

// 开硬件
extern void Thermo_HardwareOn();

// 获取数据
extern uint16 Thermo_GetValue();

// 进行标定
extern void Thermo_DoCalibration();

// 获取数据类型
extern uint8 Thermo_GetValueType();

// 设置数据类型
extern void Thermo_SetValueType(uint8 type);

// 更新当前最大值
extern uint16 Thermo_UpdateMaxValue(uint16 value);

// 在LCD上显示一个值
extern void Thermo_ShowValueOnLCD(uint8 location, uint16 value);

// 开LCD
extern void Thermo_TurnOn_LCD();

// 关LCD
extern void Thermo_TurnOff_LCD();

// 开AD
extern void Thermo_TurnOn_AD();

// 关AD
extern void Thermo_TurnOff_AD();

// 开蜂鸣器
extern void Thermo_ToneOn();

// 关蜂鸣器
extern void Thermo_ToneOff();

// 设置预测温度值
extern void Thermo_SetPreTemp(uint16 temp);

// 设置是否显示预测温度值
extern void Thermo_SetShowPreTemp(bool isShow);


#endif