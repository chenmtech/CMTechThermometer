/**************************************************************************************************
  Filename:       CMTechThermometer.h
  Revised:        $Date: 2010-08-01 14:03:16 -0700 (Sun, 01 Aug 2010) $
  Revision:       $Revision: 23256 $

  Description:    This file contains the Simple BLE Peripheral sample application
                  definitions and prototypes.

  Copyright 2010 - 2011 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef CMTECHTHERMOMETER_H
#define CMTECHTHERMOMETER_H

#ifdef __cplusplus
extern "C"
{
#endif


// CMTech Thermometer Task Events
#define TH_START_DEVICE_EVT                   0x0001     // �����豸�¼�
#define TH_PERIODIC_EVT                       0x0002     // ���ڲɼ��¶��¼� 
#define TH_CALIBRATION_EVT                    0x0004     // �궨�¼�
#define TH_SWITCH_MODE_EVT                    0x0008     // ����ģʽת���¼�
#define TH_DP_PRECAST_EVT                     0x0010     // Ԥ���¶��¼�
#define TH_DP_STABLE_EVT                      0x0020     // �����ȶ��¼�
#define TH_TONE_ON_EVT                        0x0040     // ��������
#define TH_TONE_OFF_EVT                       0x0080     // ������ͣ  
#define TH_STOP_SHOW_PRETEMP_EVT              0x0100     // ֹͣ��ʾԤ���¶�ֵ

// �������Ʊ��ֵ
#define THERMOMETER_CFG_STANDBY            0x00    //ֹͣ���²������������ģʽ
/////////////////////////////////////////////////////////////////////////////////////
// 0x01,0x02,0x03�ֱ��ʾ�ɼ������������ͣ���App_Thermometer.h�ж���
/////////////////////////////////////////////////////////////////////////////////////  
#define THERMOMETER_CFG_CALIBRATION        0x04    // ���б궨  
#define THERMOMETER_CFG_LCDON              0x05    // ��LCD
#define THERMOMETER_CFG_LCDOFF             0x06    // ��LCD


/*
 * Task Initialization for the BLE Application
 */
extern void CMTechThermometer_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 CMTechThermometer_ProcessEvent( uint8 task_id, uint16 events );



/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif 
