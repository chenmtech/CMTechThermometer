
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

// two work modes
#define MODE_ACTIVE         0x00             // active mode
#define MODE_STANDBY        0x01             // standby mode

#define STATUS_MEAS_START   0x00
#define STATUS_MEAS_STOP    0x01

/* Ative delay: 125 cycles ~1 msec */
#define ST_HAL_DELAY(n) st( { volatile uint32 i; for (i=0; i<(n); i++) { }; } )

#define DEFAULT_MEAS_INTERVAL 2

static uint8 mode = MODE_STANDBY; // current mode
static uint8 status = STATUS_MEAS_STOP;
static uint16 thermoInterval = DEFAULT_MEAS_INTERVAL; // temperature measurement interval, uint: second
static attHandleValueInd_t tempInd;

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
static uint16 gapConnHandle = INVALID_CONNHANDLE;
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

static void switchToActiveMode( void ); // switch from standby mode to active mode
static void switchToStandbyMode( void );// switch from active mode to standby mode
static void notifyTemperature(float temp); // read and process thermo-related data
static void initIOPin(); // initialize the I/O pins
static void startTempMeas( void );
static void stopTempMeas( void );


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
  
  // set temp measurement interval
  thermoInterval = 1;
  Thermometer_SetParameter( THERMOMETER_INTERVAL, sizeof(uint16), &thermoInterval ); 
  
  // Register for all key events - This app will handle all key events
  RegisterForKeys( taskID );

  //在这里初始化GPIO
  //第一：所有管脚，reset后的状态都是输入加上拉
  //第二：对于不用的IO，建议不连接到外部电路，且设为输入上拉
  //第三：对于会用到的IO，就要根据具体外部电路连接情况进行有效设置，防止耗电
  {
    // For keyfob board set GPIO pins into a power-optimized state
    // Note that there is still some leakage current from the buzzer,
    // accelerometer, LEDs, and buttons on the PCB.
    initIOPin();
  }

  Thermo_Init();

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


extern uint16 CMTechThermometer_ProcessEvent( uint8 task_id, uint16 events )
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
    
    switchToActiveMode();

    return ( events ^ TH_START_DEVICE_EVT );
  }
  
  if ( events & TH_MEAS_PERIODIC_EVT )
  {
    if(status == STATUS_MEAS_START) {
      osal_start_timerEx( taskID, TH_MEAS_PERIODIC_EVT, thermoInterval*1000L );
      
      // get value
      uint16 value = Thermo_GetValue();  
      if(value != FAILURE)
      {
        // update max value
        Thermo_UpdateMaxValue(value);          
        // show data
        Thermo_ShowValueOnLCD(1, value);
        
        if(gapRoleState == GAPROLE_CONNECTED) {
          float temp = (float)value/100.0f;
          notifyTemperature(temp);
        }
      }
    }

    return (events ^ TH_MEAS_PERIODIC_EVT);
  } 
  
  // switch work mode
  if ( events & TH_SWITCH_MODE_EVT )
  {
    if(mode == MODE_ACTIVE)
    {
      switchToStandbyMode();
    }
    else
    {
      switchToActiveMode();
    }    

    return (events ^ TH_SWITCH_MODE_EVT);
  }  
  
  // do calibration
  if ( events & TH_DO_CALIBRATION_EVT )
  {
    Thermo_DoCalibration();

    return (events ^ TH_DO_CALIBRATION_EVT);
  } 
  
  // Discard unknown events
  return 0;
}

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
  // 已连接
  if( newState == GAPROLE_CONNECTED)
  {
    // Get connection handle
    GAPRole_GetParameter( GAPROLE_CONNHANDLE, &gapConnHandle );
  }
  // 断开连接
  else if(gapRoleState == GAPROLE_CONNECTED && 
            newState != GAPROLE_CONNECTED)
  {
    stopTempMeas();
    //initIOPin();
  }
  // if started
  else if (newState == GAPROLE_STARTED)
  {
    // Set the system ID from the bd addr
    uint8 systemId[DEVINFO_SYSTEM_ID_LEN];
    GAPRole_GetParameter(GAPROLE_BD_ADDR, systemId);
    
    // shift three bytes up
    systemId[7] = systemId[5];
    systemId[6] = systemId[4];
    systemId[5] = systemId[3];
    
    // set middle bytes to zero
    systemId[4] = 0;
    systemId[3] = 0;
    
    DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);
  }
  
  gapRoleState = newState;
}

static void thermoServCB( uint8 event )
{
  switch (event)
  {
    case THERMOMETER_TEMP_IND_ENABLED:
      startTempMeas();
      break;
        
    case THERMOMETER_TEMP_IND_DISABLED:
      stopTempMeas();
      break;

    case THERMOMETER_INTERVAL_SET:
      Thermometer_GetParameter( THERMOMETER_INTERVAL, (uint8*)&thermoInterval );
      break;

    default:
      // Should not get here
      break;
  }
}

static void switchToActiveMode( void )
{  
  mode = MODE_ACTIVE;
  
  // start advertising
  uint8 advertising = TRUE;
  GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advertising );    
  
  // turn on the hardware and show the last max value
  Thermo_HardwareOn();
}

static void switchToStandbyMode( void )
{ 
  mode = MODE_STANDBY;
  
  osal_stop_timerEx( taskID, TH_MEAS_PERIODIC_EVT);
  status = STATUS_MEAS_STOP;
  
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


static void notifyTemperature(float temp)
{
  // notify temp
  uint8* p = tempInd.value;
  uint8* pTemp = (uint8*)&temp;
  *p++ = 0x00;
  *p++ = *(pTemp+3);
  *p++ = *(pTemp+2);
  *p++ = *(pTemp+1);
  *p++ = *pTemp;
  tempInd.len = 5;
  Thermometer_TempIndicate( gapConnHandle, &tempInd, taskID );
}

// 
static void startTempMeas( void )
{  
  if(status == STATUS_MEAS_STOP) {
    status = STATUS_MEAS_START;
    osal_start_timerEx( taskID, TH_MEAS_PERIODIC_EVT, thermoInterval*1000L);
  }
}

// 
static void stopTempMeas( void )
{  
  status = STATUS_MEAS_STOP;
  osal_stop_timerEx( taskID, TH_MEAS_PERIODIC_EVT ); 
}