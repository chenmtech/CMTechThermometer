
#ifndef CMTECHTHERMOMETER_H
#define CMTECHTHERMOMETER_H

// CMTech Thermometer Task Events
#define TH_START_DEVICE_EVT                   0x0001     // start device event
#define TH_MEAS_PERIODIC_EVT                  0x0002     // start periodic temp measurement event 
#define TH_SWITCH_MODE_EVT                    0x0004     // switch work mode event
#define TH_DO_CALIBRATION_EVT                 0x0008     // do calibration event


/*
 * Task Initialization for the BLE Application
 */
extern void CMTechThermometer_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 CMTechThermometer_ProcessEvent( uint8 task_id, uint16 events );


#endif 
