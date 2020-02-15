/**
 * ADS1100 header file
 * ADS1100 is an ADC with 16 bit precision
*/

#ifndef DEV_ADS1100_H
#define DEV_ADS1100_H

#include "comdef.h"
#include <iocc2541.h>


// turn on the ADS1100
extern void ADS1100_TurnOn();

// trun off the ADS1100
extern void ADS1100_TurnOff();

// initialize the ADS1100
extern void ADS1100_Init();

// get one ADC output value
// return SUCCESS or FAILURE
extern uint8 ADS1100_GetADValue(uint16 * pData);


#endif