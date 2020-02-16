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
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
#define TH_START_DEVICE_EVT                   0x0001     // 启动设备事件
#define TH_PERIODIC_EVT                       0x0002     // 周期采集温度事件 
#define TH_CALIBRATION_EVT                    0x0004     // 标定事件
#define TH_SWITCH_MODE_EVT                    0x0008     // 工作模式转换事件
#define TH_DP_PRECAST_EVT                     0x0010     // 预测温度事件
#define TH_DP_STABLE_EVT                      0x0020     // 测温稳定事件
#define TH_TONE_ON_EVT                        0x0040     // 蜂鸣器响
#define TH_TONE_OFF_EVT                       0x0080     // 蜂鸣器停  
#define TH_STOP_SHOW_PRETEMP_EVT              0x0100     // 停止显示预测温度值

// 测量控制标记值
#define THERMOMETER_CFG_STANDBY            0x00    //停止体温测量，进入待机模式
/////////////////////////////////////////////////////////////////////////////////////
// 0x01,0x02,0x03分别表示采集三种数据类型，在App_Thermometer.h中定义
/////////////////////////////////////////////////////////////////////////////////////  
#define THERMOMETER_CFG_CALIBRATION        0x04    // 进行标定  
#define THERMOMETER_CFG_LCDON              0x05    // 开LCD
#define THERMOMETER_CFG_LCDOFF             0x06    // 关LCD


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
