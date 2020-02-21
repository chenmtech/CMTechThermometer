/**
 * thermometer service header file
*/

#ifndef SERVICE_THERMOMETER_H
#define SERVICE_THERMOMETER_H

// Marks for thermometer service parameters
#define THERMOMETER_TEMP                   0       // thermometer temperature
#define THERMOMETER_TEMP_CHAR_CFG          1       // thermometer temperature ccc
#define THERMOMETER_TYPE                   2       // temperature type, which means the location of the sensor on the body 
#define THERMOMETER_INTERVAL               3       // measurement interval
#define THERMOMETER_IRANGE                 4       // interval range: [min, max]

// UUID of the service, characteristics and descriptors
#define THERMOMETER_SERV_UUID    0x1809
#define THERMOMETER_TEMP_UUID    0x2A1C
#define THERMOMETER_TYPE_UUID    0x2A1D
#define THERMOMETER_INTERVAL_UUID    0x2A21
#define THERMOMETER_IRANGE_UUID 0x2906

// Position in service attribute table
#define THERMOMETER_TEMP_VALUE_POS            2  // temperature measurement value position

// Values for sensor location
#define THERMOMETER_TYPE_ARMPIT            0x01
#define THERMOMETER_TYPE_BODY              0x02
#define THERMOMETER_TYPE_EAR               0x03
#define THERMOMETER_TYPE_FINGER            0x04
#define THERMOMETER_TYPE_GASTRO            0x05
#define THERMOMETER_TYPE_MOUTH             0x06
#define THERMOMETER_TYPE_RECTUM            0x07
#define THERMOMETER_TYPE_TOE               0x08
#define THERMOMETER_TYPE_TYMPNUM           0x09

// Service bit field
#define THERMOMETER_SERVICE               0x00000001

// Callback events
#define THERMOMETER_TEMP_IND_ENABLED          0
#define THERMOMETER_TEMP_IND_DISABLED         1
#define THERMOMETER_INTERVAL_SET              2


// Thermometer Interval Range Struct
typedef struct
{
  uint16 low;         
  uint16 high; 
} thermoIRange_t;


typedef NULL_OK void (*thermoServCB_t)( uint8 paramID );

typedef struct
{
  thermoServCB_t        pfnThermoServCB;  // Called when characteristic value changes
} thermoServCBs_t;


// Add Service
extern bStatus_t Thermometer_AddService( uint32 services );

// Register service callback
extern bStatus_t Thermometer_RegisterAppCBs( thermoServCBs_t *appCBs );

// set the service parameter
extern bStatus_t Thermometer_SetParameter( uint8 param, uint8 len, void *value );

// set the service parameter
extern bStatus_t Thermometer_GetParameter( uint8 param, void *value );

// indicate thermometer temperature measurement value
extern bStatus_t Thermometer_TempIndicate( uint16 connHandle, attHandleValueInd_t * pIndi, uint8 taskId );

#endif












