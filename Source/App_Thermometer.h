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


// ��ʼ��
extern void Thermo_Init();

// ��Ӳ��
extern void Thermo_HardwareOff();

// ��Ӳ��
extern void Thermo_HardwareOn();

// ��ȡ����
extern uint16 Thermo_GetValue();

// ���б궨
extern void Thermo_DoCalibration();

// ��ȡ��������
extern uint8 Thermo_GetValueType();

// ������������
extern void Thermo_SetValueType(uint8 type);

// ���µ�ǰ���ֵ
extern uint16 Thermo_UpdateMaxValue(uint16 value);

// ��LCD����ʾһ��ֵ
extern void Thermo_ShowValueOnLCD(uint8 location, uint16 value);

// ��LCD
extern void Thermo_TurnOn_LCD();

// ��LCD
extern void Thermo_TurnOff_LCD();

// ��AD
extern void Thermo_TurnOn_AD();

// ��AD
extern void Thermo_TurnOff_AD();

// ��������
extern void Thermo_ToneOn();

// �ط�����
extern void Thermo_ToneOff();

// ����Ԥ���¶�ֵ
extern void Thermo_SetPreTemp(uint16 temp);

// �����Ƿ���ʾԤ���¶�ֵ
extern void Thermo_SetShowPreTemp(bool isShow);


#endif