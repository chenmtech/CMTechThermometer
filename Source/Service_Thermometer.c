
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "cmutil.h"

#include "Service_Thermometer.h"


// 
CONST uint8 thermoServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(THERMOMETER_SERV_UUID), HI_UINT16(THERMOMETER_SERV_UUID)
};

CONST uint8 thermoTempUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(THERMOMETER_TEMP_UUID), HI_UINT16(THERMOMETER_TEMP_UUID)
};

CONST uint8 thermoTypeUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(THERMOMETER_TYPE_UUID), HI_UINT16(THERMOMETER_TYPE_UUID)
};

// measurement interval UUID
CONST uint8 thermoIntervalUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(THERMOMETER_INTERVAL_UUID), HI_UINT16(THERMOMETER_INTERVAL_UUID)
};

// interval range UUID
CONST uint8 thermoIRangeUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(THERMOMETER_IRANGE_UUID), HI_UINT16(THERMOMETER_IRANGE_UUID)
};

// 
static CONST gattAttrType_t thermoService = { ATT_BT_UUID_SIZE, thermoServUUID };


static uint8 thermoTempProps = GATT_PROP_INDICATE;
static uint8 thermoTemp = 0;
static gattCharCfg_t thermoTempConfig[GATT_MAX_NUM_CONN];

static uint8 thermoTypeProps = GATT_PROP_READ;
static uint8 thermoType = 0;    

// interval，unit: second
static uint8 thermoIntervalProps = GATT_PROP_READ | GATT_PROP_WRITE;
static uint16 thermoInterval = 2;  

// interval range
static thermoIRange_t  thermoIRange = {1,60};


// 服务的属性表
static gattAttribute_t thermoServAttrTbl[] = 
{
  // thermo Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&thermoService                   /* pValue */
  },

    // 1. TEMPERATURE MEASUREMENT
    // Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &thermoTempProps 
    },

      // Characteristic Value
      { 
        { ATT_BT_UUID_SIZE, thermoTempUUID },
        GATT_PERMIT_READ, 
        0, 
        &thermoTemp 
      }, 
      
      // Characteristic configuration
      {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)thermoTempConfig
      },
      
    // 2. TEMPERATURE TYPE  
    // Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &thermoTypeProps
    },

      // Characteristic Value
      {
        { ATT_BT_UUID_SIZE, thermoTypeUUID },
        GATT_PERMIT_READ,
        0,
        &thermoType
      },

    // 3. MEASUREMENT INTERVAL
    // Characteristic Declaration 
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &thermoIntervalProps
    },

      // Characteristic Value
      {
        { ATT_BT_UUID_SIZE, thermoIntervalUUID },
        GATT_PERMIT_READ | GATT_PERMIT_AUTHEN_WRITE,
        0,
        (uint8*)&thermoInterval
      },
      
      // Characteristic Descriptor
      { 
        { ATT_BT_UUID_SIZE, thermoIRangeUUID },
        GATT_PERMIT_READ,
        0, 
        (uint8 *)&thermoIRange 
      },
};

// 
static thermometerServiceCBs_t *appCBs = NULL;

static uint8 readAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
static bStatus_t writeAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );

CONST gattServiceCBs_t servCBs =
{
  readAttrCB,      // Read callback function pointer
  writeAttrCB,     // Write callback function pointer
  NULL             // Authorization callback function pointer
};


static void handleConnStatusCB( uint16 connHandle, uint8 changeType );


extern bStatus_t Thermometer_AddService( uint32 services )
{
  uint8 status = SUCCESS;

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, thermoTempConfig );

  // Register with Link DB to receive link status change callback
  VOID linkDB_Register( handleConnStatusCB );  
  
  if ( services & THERMOMETER_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( thermoServAttrTbl, 
                                          GATT_NUM_ATTRS( thermoServAttrTbl ),
                                          &servCBs );
  }

  return ( status );
}


// 登记应用层给的回调
extern bStatus_t Thermometer_RegisterAppCBs( thermometerServiceCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    appCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

extern bStatus_t Thermometer_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case THERMOMETER_TYPE:
      thermoType = *((uint8*)value);
      break;
      
    case THERMOMETER_INTERVAL:
      osal_memcpy( (uint8*)&thermoInterval, value, len );
      break;      
 
    case THERMOMETER_TEMP_CHAR_CFG:      
      // Need connection handle
      //thermometerTempConfig.value = *((uint8*)value);
      break;  
      
    case THERMOMETER_IRANGE:      
      thermoIRange = *((thermoIRange_t*)value);
      break;       
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

extern bStatus_t Thermometer_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
    case THERMOMETER_TYPE:
      *((uint8*)value) = thermoType;
      break;

    case THERMOMETER_INTERVAL:
      osal_memcpy( value, (uint8*)&thermoInterval, 2 );
      break;
    
    case THERMOMETER_IRANGE:
      *((thermoIRange_t*)value) = thermoIRange;
      break;
      
  case THERMOMETER_TEMP_CHAR_CFG:
      // Need connection handle
      //*((uint16*)value) = thermometerTempConfig.value;
      break;   
            
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

extern bStatus_t Thermometer_TempIndicate( uint16 connHandle, attHandleValueInd_t * pIndi, uint8 taskId )
{
  uint16 value = GATTServApp_ReadCharCfg( connHandle, thermoTempConfig );

  // If indications enabled
  if ( value & GATT_CLIENT_CFG_INDICATE )
  {
    // Set the handle (uses stored relative handle to lookup actual handle)
    pIndi->handle = thermoServAttrTbl[THERMOMETER_TEMP_VALUE_POS].handle;
  
    // Send the Indication
    return GATT_Indication( connHandle, pIndi, FALSE, taskId );
  }

  return bleIncorrectMode;
}


static uint8 readAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen )
{
  bStatus_t status = SUCCESS;
  uint16 uuid;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
  
  // 16-bit UUID
  uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
  
  switch ( uuid )
  {
    case THERMOMETER_TYPE_UUID:
        *pLen = 1;
        *pValue = thermoType;
        break;
        
      case THERMOMETER_INTERVAL_UUID:
        *pLen = 2;
        VOID osal_memcpy( pValue, &thermoInterval, 2 ) ;
        break;

      case THERMOMETER_IRANGE_UUID:
        *pLen = 4;
         VOID osal_memcpy( pValue, &thermoIRange, 4 ) ;
        break;        
        
      default:
        *pLen = 0;
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
  }  
  
  return status;
}


static bStatus_t writeAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset )
{
  bStatus_t status = SUCCESS;

  uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
  switch ( uuid )
  {
    case  GATT_CLIENT_CHAR_CFG_UUID:
      // Validate/Write Temperature measurement setting
      status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_INDICATE );
      if ( status == SUCCESS )
      {
        uint16 value = BUILD_UINT16( pValue[0], pValue[1] );      
      
        (appCBs->pfnThermometerServiceCB)( (value == GATT_CFG_NO_OPERATION) ? 
                     THERMOMETER_TEMP_IND_DISABLED :
                     THERMOMETER_TEMP_IND_ENABLED );
      }
      break;
  
    case THERMOMETER_INTERVAL_UUID:
      // Validate the value
      // Make sure it's not a blob oper
      if ( offset == 0 )
      {
        if ( len != 2 )
          status = ATT_ERR_INVALID_VALUE_SIZE;
      }
      else
      {
        status = ATT_ERR_ATTR_NOT_LONG;
      }
      
      //validate range
      if ((*pValue >= thermoIRange.high) | ((*pValue <= thermoIRange.low) & (*pValue != 0)))
      {
        status = ATT_ERR_INVALID_VALUE;
      }
      
      //Write the value
      if ( status == SUCCESS )
      {
        uint8 *pCurValue = (uint8 *)pAttr->pValue;        
        // *pCurValue = *pValue; 
        VOID osal_memcpy( pCurValue, pValue, 2 ) ;
        
        //notify application of write
        (appCBs->pfnThermometerServiceCB)(THERMOMETER_INTERVAL_SET);
        
      }
      break;
    
    default:
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }
  
  return ( status );
}

static void handleConnStatusCB( uint16 connHandle, uint8 changeType )
{ 
  // Make sure this is not loopback connection
  if ( connHandle != LOOPBACK_CONNHANDLE )
  {
    // Reset Client Char Config if connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
         ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) && 
           ( !linkDB_Up( connHandle ) ) ) )
    { 
      GATTServApp_InitCharCfg( connHandle, thermoTempConfig );
    }
  }
}