/**************************************************************************************************
  Filename:       CMTechThermometer.c
  Revised:        $Date: 2010-08-06 08:56:11 -0700 (Fri, 06 Aug 2010) $
  Revision:       $Revision: 23333 $

  Description:    This file contains the Simple BLE Peripheral sample application
                  for use with the CC2540 Bluetooth Low Energy Protocol Stack.

  Copyright 2010 - 2013 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "osal_snv.h"

#include "OnBoard.h"
//#include "hal_adc.h"
#include "hal_key.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "Service_Thermometer.h"

#if defined ( PLUS_BROADCASTER )
  #include "peripheralBroadcaster.h"
#else
  #include "peripheral.h"
#endif

#include "gapbondmgr.h"
#include "App_Thermometer.h"
//#include "App_DataProcessor.h"
#include "CMTechThermometer.h"

#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif

#define INVALID_CONNHANDLE                    0xFFFF

#if defined ( PLUS_BROADCASTER )
  #define ADV_IN_CONN_WAIT                    500 // delay 500 ms
#endif

// two modes
#define MODE_ACTIVE         0             // active mode
#define MODE_STANDBY        1             // standby mode
#define THERMO_SHOWLASTMAXTEMP_TIME           3000 // the time that showing the last max temp, unit: second
#define THERMO_SHOWPRETEMP_TIME               20000 // the time that showing the predicted temp

/* Ative delay: 125 cycles ~1 msec */
#define ST_HAL_DELAY(n) st( { volatile uint32 i; for (i=0; i<(n); i++) { }; } )


static uint8 curMode = MODE_STANDBY; // current mode
static bool ADCEnabled = FALSE; // is the ADC enabled
static uint16 thermoInterval = 1; // temperature measurement interval, uint: second

// advertise data
static uint8 advertData[] = 
{ 
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID
  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,
  LO_UINT16( THERMOMETER_SERV_UUID ),
  HI_UINT16( THERMOMETER_SERV_UUID ),

};

// scan response data
static uint8 scanResponseData[] =
{
  0x06,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_SHORT,   
  'C',
  'M',
  '_',
  'T',
  'M'
};

// GGS device name
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "CM Thermometer";

static uint8 taskID;   // Task ID for internal task/event processing
static gaprole_States_t gapRoleState = GAPROLE_INIT;

static void processOSALMsg( osal_event_hdr_t *pMsg );
static void handleKeys( uint8 shift, uint8 keys );
static void gapRoleStateCB( gaprole_States_t newState );
static void thermoServCB( uint8 event );

// GAP Role Callbacks
static gapRolesCBs_t gapRoleStateCBs =
{
  gapRoleStateCB,  // Profile State Change Callbacks
  NULL             // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t bondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};

static thermometerServiceCBs_t thermoServCBs =
{
  thermoServCB    // Charactersitic value change callback
};

static void initIntoStandbyMode(); // initialize into the standby mode
static void switchFromStandbyToActive( void ); // switch from standby mode to active mode
static void switchFromActiveToStandby( void );// switch from active mode to standby mode
static void readAndProcessThermoData(); // read and process thermo-related data
static void initIOPin(); // initialize the I/O pins


extern void CMTechThermometer_Init( uint8 task_id )
{
  taskID = task_id;
  
  // Setup the GAP Peripheral Role Profile
  {
    // set the advertising data and scan response data
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );
    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanResponseData ), scanResponseData );
    
    // set the advertising parameters
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, 1600 ); // units of 0.625ms
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, 1600 ); // units of 0.625ms
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_MIN, 0 ); // advertising forever
    
    // disable advertising
    uint8 advertising = FALSE;
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advertising );
    
    // set the pause time from the connection and the update of the connection parameters
    // during the time, client can finish the tasks e.g. service discovery 
    // the unit of time is second
    GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, 2 ); 
    
    // set the connection parameter
    uint16 desired_min_interval = 200;  // units of 1.25ms 
    uint16 desired_max_interval = 1600; // units of 1.25ms
    uint16 desired_slave_latency = 1;
    uint16 desired_conn_timeout = 1000; // units of 10ms
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
    uint8 enable_update_request = TRUE;
    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
  }
  
  // set GGS device name
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }  

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  Thermometer_AddService( GATT_ALL_SERVICES );
  
#if defined FEATURE_OAD
  VOID OADTarget_AddService();                    // OAD Profile
#endif
  
  Thermometer_RegisterAppCBs( &thermoServCBs );

  RegisterForKeys( taskID );

  //在这里初始化GPIO
  //第一：所有管脚，reset后的状态都是输入加上拉
  //第二：对于不用的IO，建议不连接到外部电路，且设为输入上拉
  //第三：对于会用到的IO，就要根据具体外部电路连接情况进行有效设置，防止耗电
  {
    // For keyfob board set GPIO pins into a power-optimized state
    // Note that there is still some leakage current from the buzzer,
    // accelerometer, LEDs, and buttons on the PCB.
    
    // Register for all key events - This app will handle all key events
    
    initIOPin();
  }

  Thermo_Init();
  
  // 初始化为待机模式
  initIntoStandbyMode();  
 
  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );  

  // Setup a delayed profile startup
  osal_set_event( taskID, TH_START_DEVICE_EVT );
}


// 初始化IO管脚
static void initIOPin()
{
  // 全部设为GPIO
  P0SEL = 0; // Configure Port 0 as GPIO
  P1SEL = 0; // Configure Port 1 as GPIO
  P2SEL = 0; // Configure Port 2 as GPIO
//
//  // 除了P0.0和P0.1配置为button输入高电平，其他全部设为输出低电平
  P0DIR = 0xFC; // Port 0 pins P0.0 and P0.1 as input (buttons),
//                // all others (P0.1-P0.7) as output
  P1DIR = 0xFF; // All port 1 pins (P1.0-P1.7) as output
  P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output
//  
  P0 = 0x03; // All pins on port 0 to low except for P0.0 and P0.1(buttons)
  P1 = 0;   // All pins on port 1 to low
  P2 = 0;   // All pins on port 2 to low   
  
  // I2C的SDA, SCL设置为GPIO, 输出低电平，否则功耗很大
  I2CWC = 0x83;
  I2CIO = 0x00;
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 CMTechThermometer_ProcessEvent( uint8 task_id, uint16 events )
{

  VOID task_id; // OSAL required parameter that isn't used in this function

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( taskID )) != NULL )
    {
      processOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & TH_START_DEVICE_EVT )
  {    
    // Start the Device
    VOID GAPRole_StartDevice( &gapRoleStateCBs );

    // Start Bond Manager
    VOID GAPBondMgr_Register( &bondMgrCBs );
    
    switchFromStandbyToActive();

    return ( events ^ TH_START_DEVICE_EVT );
  }
  
  if ( events & TH_PERIODIC_EVT )
  {
    if(ADCEnabled)
    {
      readAndProcessThermoData();
      osal_start_timerEx( taskID, TH_PERIODIC_EVT, thermoInterval*1000L );
    }
    else
    {
      // 停止AD采集
      //Thermo_TurnOff_AD();
    }

    return (events ^ TH_PERIODIC_EVT);
  }

  
  // 标定
  if ( events & TH_CALIBRATION_EVT )
  {
    Thermo_DoCalibration();

    return (events ^ TH_CALIBRATION_EVT);
  }  
  
  // 切换工作模式
  if ( events & TH_SWITCH_MODE_EVT )
  {
    if(curMode == MODE_ACTIVE)
    {
      switchFromActiveToStandby();
    }
    else
    {
      switchFromStandbyToActive();
    }    

    return (events ^ TH_SWITCH_MODE_EVT);
  }  
  
  // Discard unknown events
  return 0;
}




/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void processOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
    case KEY_CHANGE:
      handleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
      break;

    default:
      // do nothing
      break;
  }
}

/*********************************************************************
 * @fn      CMTechThermometer_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void handleKeys( uint8 shift, uint8 keys )
{
  VOID shift;  // Intentionally unreferenced parameter

  if ( keys & HAL_KEY_SW_1 )
  {
    osal_set_event( taskID, TH_SWITCH_MODE_EVT);

  } 
}

static void gapRoleStateCB( gaprole_States_t newState )
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);
      }
      break;

    case GAPROLE_ADVERTISING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Advertising",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_CONNECTED:


      break;

    case GAPROLE_WAITING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Disconnected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
          
        // 断开连接时，停止AD采集
        //Thermo_TurnOff_AD();
      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Timed Out",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_ERROR:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Error",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    default:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

  }
  
  gapRoleState = newState;

}


static void thermoServCB( uint8 event )
{
  uint8 newValue;

  switch (event)
  {
    case THERMOMETER_CONF:
      Thermometer_GetParameter( THERMOMETER_CONF, &newValue );
      
      if ( newValue == THERMOMETER_CONF_STANDBY)  // 停止采集，进入待机模式
      {
        switchFromActiveToStandby();
      }
      
      else if ( newValue == THERMOMETER_CONF_CALIBRATION) // 进行标定
      {
        osal_set_event( taskID, TH_CALIBRATION_EVT);
      }
      
      else if ( newValue == THERMOMETER_CONF_LCDON) // 开LCD
      {
        Thermo_TurnOn_LCD();
      }   
      
      else if ( newValue == THERMOMETER_CONF_LCDOFF) // 关LCD
      {
        Thermo_TurnOff_LCD();
      }  
      
      else // 剩下的就是设置数据类型
      { 
        Thermo_SetValueType(newValue);
      }
      
      break;

    case THERMOMETER_INTERVAL_SET:
      Thermometer_GetParameter( THERMOMETER_INTERVAL, (uint8*)&thermoInterval );
      break;

    default:
      // Should not get here
      break;
  }
}


// 初始化为待机模式
static void initIntoStandbyMode()
{
  curMode = MODE_STANDBY;
  
  // 采样周期为1秒
  thermoInterval = 1;
    
  // 设置传输周期
  Thermometer_SetParameter( THERMOMETER_INTERVAL, sizeof(uint16), &thermoInterval ); 
  
  // 停止采样
  ADCEnabled = FALSE;
}


static void switchFromStandbyToActive( void )
{  
  curMode = MODE_ACTIVE;
  
  // 开硬件
  Thermo_HardwareOn();
  
  // 开始广播
  uint8 advertising = TRUE;
  GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advertising );  
  
  // 显示上次最大值后，开始AD采样
  ADCEnabled = TRUE;
  osal_start_timerEx( taskID, TH_PERIODIC_EVT, THERMO_SHOWLASTMAXTEMP_TIME);
}


static void switchFromActiveToStandby( void )
{ 
  curMode = MODE_STANDBY;
  
  // 停止AD
  ADCEnabled = FALSE;
  osal_stop_timerEx( taskID, TH_PERIODIC_EVT);
  
  // 终止蓝牙连接
  if ( gapRoleState == GAPROLE_CONNECTED )
  {
    GAPRole_TerminateConnection( );
  }
  
  // 停止广播
  uint8 advertising = FALSE;
  GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advertising );    
  
  // 关硬件
  Thermo_HardwareOff();
  
  initIOPin();
}


static void readAndProcessThermoData()
{
  // 获取数据
  uint16 value = Thermo_GetValue();  
  
  // 错误数据，不发送，不显示
  if(value == FAILURE) return;   
  
  // 更新数据最大值
  Thermo_UpdateMaxValue(value);  
  
  // 在液晶屏上显示数据
  Thermo_ShowValueOnLCD(1, value);
  
  Thermometer_SetParameter( THERMOMETER_DATA, THERMOMETER_DATA_LEN, (uint8*)&value); 
}