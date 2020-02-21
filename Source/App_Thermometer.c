
#include "bcomdef.h"
#include "osal_snv.h"
#include "Dev_ADS1100.h"
#include "Dev_HT1621B.h"
#include "App_Thermometer.h"

// NVID
#define NVID_CALI_VALUE 0x80      // the NVID of the "caliValue"
#define NVID_VALUE_TYPE 0x81      // the NVID of the value type, 1:AD 2:Resistor 3:Temperature
#define NVID_MAX_VALUE  0x82      // the NVID of the last max value

/**
 * 37�ȵ�ʱ���ADֵ�����ڱ궨��
 * �궨������ʹһ���µ��豸�в�37���¶ȣ��õ������ADֵ
 * Ȼ���ADVALUE37�Ƚϣ��õ�������豸��caliValue�������浽NV��
 * ������ʼ����ʱ�����caliValue,ÿ�εõ�һ�����ADֵ������ȥcaliValue���Ϳ�����ɱ궨
*/
#define ADVALUE37   17261   

/**
 * ����ADS1100��Offset Error�����������Ҫ���б궨������Ǳ궨ֵ
 * Ŀǰ��1���������˵İ壩�ı궨ֵΪ0����������ݱ��Ǹ��ݰ�1�����ģ�
 * ��2�ı궨ֵΪ14������2�����ֵ�Ȱ�1Ҫ��14��
 * ���ֵ�ᱣ�浽NV�У�ÿ������ʱ���NV������
*/
static int16  caliValue = 0;    

//////////////////����Ϊ����NTCС��ͷ�Ĳ���
//��33���϶�-44���϶ȣ�0.1���϶�Ϊ���
//���Ǵ�35-38���϶ȣ�0.05���϶�Ϊ���
static const uint16 TTable[] = 
{
  3300,3310,3320,3330,3340,3350,3360,3370,3380,3390,      //33���϶�
  3400,3410,3420,3430,3440,3450,3460,3470,3480,3490,      //34���϶�
  3500,3505,3510,3515,3520,3525,3530,3535,3540,3545,      //35.00-35.45���϶�
  3550,3555,3560,3565,3570,3575,3580,3585,3590,3595,      //35.50-35.95���϶�
  3600,3605,3610,3615,3620,3625,3630,3635,3640,3645,      //36.00-36.45���϶�
  3650,3655,3660,3665,3670,3675,3680,3685,3690,3695,      //36.50-36.95���϶�
  3700,3705,3710,3715,3720,3725,3730,3735,3740,3745,      //37.00-37.45���϶�
  3750,3755,3760,3765,3770,3775,3780,3785,3790,3795,      //37.50-37.95���϶�
  3800,3810,3820,3830,3840,3850,3860,3870,3880,3890,      //38���϶�
  3900,3910,3920,3930,3940,3950,3960,3970,3980,3990,      //39���϶�
  4000,4010,4020,4030,4040,4050,4060,4070,4080,4090,      //40���϶�     
  4100,4110,4120,4130,4140,4150,4160,4170,4180,4190,      //41���϶� 
  4200,4210,4220,4230,4240,4250,4260,4270,4280,4290,      //42���϶� 
  4300,4310,4320,4330,4340,4350,4360,4370,4380,4390,      //43���϶�    
  4400  //44���϶�
};

//��Ӧ�¶��µ�NTC����ֵ���ɻ����ṩ�ı��õ�
static const uint16 RTable[] = 
{
  34949,34804,34659,34515,34372,34230,34088,33947,33806,33666,      //33���϶�
  33527,33388,33250,33113,32976,32840,32704,32570,32435,32302,      //34���϶�
  32168,32102,32036,31970,31904,31838,31773,31707,31642,31577,      //35.00-35.45���϶�
  31512,31447,31382,31318,31253,31189,31125,31061,30997,30934,      //35.50-35.95���϶�
  30870,30807,30743,30680,30617,30555,30492,30429,30367,30305,      //36.00-36.45���϶�
  30243,30181,30119,30057,29995,29934,29873,29812,29751,29690,      //36.50-36.95���϶�
  29629,29568,29508,29448,29387,29327,29267,29208,29148,29088,      //37.00-37.45���϶�
  29029,28970,28911,28852,28793,28734,28676,28617,28559,28501,      //37.50-37.95���϶�
  28442,28327,28211,28097,27983,27869,27756,27643,27531,27419,      //38���϶�
  27308,27197,27087,26977,26868,26759,26651,26543,26436,26329,      //39���϶�
  26223,26117,26012,25907,25802,25698,25595,25491,25389,25287,      //40���϶�     
  25185,25084,24983,24882,24782,24683,24584,24485,24387,24289,      //41���϶� 
  24192,24095,23999,23903,23807,23712,23617,23523,23429,23335,      //42���϶� 
  23242,23149,23057,22965,22873,22782,22692,22601,22511,22422,      //43���϶�    
  22333  //44���϶�
};

//��Ӧ�����µ�AD���ֵ����ʵ��õ�
// 33.0-33.9֮������ݲ���ʵ������ֻ�ǹ���ģ���Ϊ��ʱû����33-34������
static const uint16 ADTable[] = 
{
  15907,15941,15975,16009,16043,16077,16111,16145,16179,16213,      //33���϶�
  16247,16281,16316,16349,16384,16419,16452,16486,16520,16554,      //34���϶�
  16588,16605,16621,16639,16655,16671,16689,16706,16723,16740,      //35.00-35.45���϶�
  16756,16773,16789,16807,16824,16840,16857,16874,16891,16907,      //35.50-35.95���϶�
  16925,16942,16958,16976,16992,17009,17026,17042,17059,17075,      //36.00-36.45���϶�
  17092,17110,17127,17144,17161,17177,17194,17211,17228,17245,      //36.50-36.95���϶�
  17261,17278,17295,17311,17327,17344,17361,17378,17394,17411,      //37.00-37.45���϶�
  17427,17444,17461,17477,17494,17511,17528,17544,17560,17577,      //37.50-37.95���϶�
  17594,17627,17660,17693,17726,17759,17793,17825,17858,17892,      //38���϶�
  17925,17958,17990,18023,18057,18090,18122,18155,18188,18221,      //39���϶�
  18254,18286,18318,18351,18384,18416,18448,18481,18514,18546,      //40���϶�
  18579,18612,18644,18677,18709,18741,18773,18806,18837,18869,      //41���϶�
  18901,18933,18966,18997,19030,19062,19094,19125,19158,19189,      //42���϶�
  19221,19253,19284,19316,19348,19379,19410,19443,19474,19505,      //43���϶�
  19536       //44���϶�
};

static uint8 LEN = sizeof(ADTable)/sizeof(uint16); // table length
static uint8 idx = 0; // data index in the table
static uint8 valueType = THERMOMETER_CFG_VALUETYPE_T;  // value type
static uint16 valueLowLimit = LOWLIMIT_T; // low limit of value
static uint16 valueUpLimit = UPLIMIT_T; // up limit of value
static uint16 maxValue = 0; // max value measured in this time
static uint16 curValue = 65535;   // current value measured. 65535 which is a value that can't reached.

// Ŀǰ�����ĸ߾��Ȳο����趼��33009Ohm
//static uint16 RREF = 33009;   // �߾��Ȳο����裬��λOhm

// calculate R from AD
static uint16 calcRFromADValue(uint16 ADValue);
// calculate T from AD
static uint16 calcTFromADValue(uint16 ADValue);
// read the last max value from NV
static uint16 readLastMaxValueFromNV();
// get calibrated AD value
static uint16 getCaliADValue();
// get T
static uint16 getTemperature();
// get R
static uint16 getResistor();


// initialize the model
extern void Thermo_Init()
{
  // initialize the HT1621 to power down state
  HT1621B_InitToPowerDown();  
  // initialize the ADS1100
  ADS1100_Init();    
  // read the caliValue from NV
  uint8 rtn = osal_snv_read(NVID_CALI_VALUE, sizeof(int16), (uint8*)&caliValue);
  if(rtn != SUCCESS)
    caliValue = 0;    
}

// save the max value and then turn off the hardware, including the LCD and ADC
extern void Thermo_HardwareOff()
{  
  // save valueType and max value into NV
  osal_snv_write(NVID_VALUE_TYPE, sizeof(uint8), (uint8*)&valueType);
  osal_snv_write(NVID_MAX_VALUE, sizeof(uint16), (uint8*)&maxValue); 
  // clear the LCD
  HT1621B_ClearLCD();    
  // close the LCD
  HT1621B_TurnOffLCD();    
  // close the AD
  ADS1100_TurnOff(); 
}

// turn on the hardware, including the LCD and ADC, and then read the last max value and show it.
extern void Thermo_HardwareOn()
{
  // turn on the LCD
  HT1621B_TurnOnLCD();  
  // clear the LCD
  HT1621B_ClearLCD();   
  // turn on the AD
  ADS1100_TurnOn();  
  // read the last max value from NV
  uint16 lastMaxValue = readLastMaxValueFromNV();
  //show the last max value on the LCD
  Thermo_ShowValueOnLCD(1, lastMaxValue); 
  
  maxValue = 0;  
  curValue = 65535; 
}

// get value which can be AD, R or T value according to the value type
extern uint16 Thermo_GetValue()
{
  switch(valueType)
  {
  case THERMOMETER_CFG_VALUETYPE_AD:
    return getCaliADValue();
  case THERMOMETER_CFG_VALUETYPE_R:
    return getResistor();
  case THERMOMETER_CFG_VALUETYPE_T:
    return getTemperature();
  }
  return FAILURE;
}

// do calibration experiment for a new device in order to get its caliValue
extern void Thermo_DoCaliExperiment()
{
  uint16 sum = 0;
  uint16 data = 0;
  
  for(int i = 0; i < 3; i++)
  {
    ADS1100_GetADValue(&data);
    sum += data;
  }
  
  // ȡ37��ʱ������AD���ƽ��ֵ�����׼��37�����ֵ���бȽϣ��õ��궨���ֵ
  caliValue = (int16)((double)sum/3 - ADVALUE37 + 0.5);  // ��������
  
  // save the "caliValue" into NV
  osal_snv_write(NVID_CALI_VALUE, sizeof(int16), (uint8*)&caliValue);
}

// update the maxValue
extern uint16 Thermo_UpdateMaxValue(uint16 value)
{
  if(maxValue < value) maxValue = value;
  return maxValue;
}

// show a value on the LCD
extern void Thermo_ShowValueOnLCD(uint8 location, uint16 value)
{ 
  // if the value is equal to the curValue, return.
  if( curValue == value ) return;
  
  curValue = value;
  if(value <= valueLowLimit)    //��ʾ"L"
  {
    HT1621B_ShowL(location);
    return;
  }
  else if(value >= valueUpLimit)   //��ʾ"H"
  {
    HT1621B_ShowH(location);
    return;
  }    
  else
  {
    // show Temperature
    if(valueType == THERMOMETER_CFG_VALUETYPE_T)
    {
      HT1621B_ShowTemperature(location, value, FALSE);
    }
    else  // show other value with non-temperature type
      HT1621B_ShowUint16Data(location, value);  
  }
}

// get calibrated AD value
static uint16 getCaliADValue()
{
  // get AD value
  uint16 ADValue = 0;
  if(ADS1100_GetADValue(&ADValue) == FAILURE) return FAILURE;
  
  // calibrate AD value
  return (uint16)((double)ADValue - caliValue);  
}

// get Temperature value
static uint16 getTemperature()
{
  uint16 ADValue = getCaliADValue();
  if(ADValue == FAILURE) return FAILURE;
  
  return calcTFromADValue(ADValue);
}

// get R
static uint16 getResistor()
{
  uint16 ADValue = getCaliADValue();
  if(ADValue == FAILURE) return FAILURE;
  
  return calcRFromADValue(ADValue);
}

static uint16 readLastMaxValueFromNV()
{
  // read valueType from NV
  uint8 rtn = osal_snv_read(NVID_VALUE_TYPE, sizeof(uint8), (uint8*)&valueType);
  if(rtn != SUCCESS)
    valueType = THERMOMETER_CFG_VALUETYPE_T;    
  
  // read the last max value from NV
  uint16 lastMaxValue = 0;
  rtn = osal_snv_read(NVID_MAX_VALUE, sizeof(uint16), (uint8*)&lastMaxValue);
  if(rtn != SUCCESS)
    lastMaxValue = 0;   
  
  // set the value limit
  switch(valueType)
  {
  case THERMOMETER_CFG_VALUETYPE_AD:
    valueLowLimit = LOWLIMIT_AD;
    valueUpLimit = UPLIMIT_AD;
    break;
  case THERMOMETER_CFG_VALUETYPE_R:
    valueLowLimit = LOWLIMIT_R;
    valueUpLimit = UPLIMIT_R;
    break;
  case THERMOMETER_CFG_VALUETYPE_T:
    valueLowLimit = LOWLIMIT_T;
    valueUpLimit = UPLIMIT_T;
    break;
  }
  return lastMaxValue;
}

// ��AD���ֵ�������ֵ
static uint16 calcRFromADValue(uint16 ADValue)
{
  if(ADValue < ADTable[0])
  {
    idx = 0;
    return UPLIMIT_R;
  } 
  
  if(ADValue > ADTable[LEN-1])
  {
    idx = LEN-1;
    return LOWLIMIT_R;    
  }
  
  while(ADTable[idx] > ADValue)
  {
    idx--;      
  }
  
  while(ADTable[idx+1] < ADValue)
  {
    idx++;
  }

  uint16 result = (uint16)( RTable[idx] - ((double)(RTable[idx]-RTable[idx+1]))/(ADTable[idx+1]-ADTable[idx])*(ADValue-ADTable[idx]) );
  
  return result;  
}

// ��AD���ֵ�������¶�ֵ
static uint16 calcTFromADValue(uint16 ADValue)
{
  if(ADValue < ADTable[0])
  {
    idx = 0;
    return LOWLIMIT_T;
  } 
  
  if(ADValue > ADTable[LEN-1])
  {
    idx = LEN-1;
    return UPLIMIT_T;    
  }
  
  while(ADTable[idx] > ADValue)
  {
    idx--;      
  }
  
  while(ADTable[idx+1] < ADValue)
  {
    idx++;
  }
  
  uint16 result = (uint16)( ((double)(TTable[idx+1]-TTable[idx]))/(ADTable[idx+1]-ADTable[idx])*(ADValue-ADTable[idx])+TTable[idx] );
  
  return result;  
}