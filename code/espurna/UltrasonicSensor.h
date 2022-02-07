#include "Arduino.h"
#include "espurna.h"
#include "SoftwareSerial.h"
#include "ModbusRTU.h"
#include "Modbus.h"

#include "terminal_commands.h"

#define Rxpin           14
#define Txpin           12


void UltrasonicSensor_Setup();


typedef union{
    float data_f;
    struct 
    {
        unsigned int mantissa : 23;
        unsigned int exponent : 8;
        unsigned int sign     : 1;
    }Raw;
    
}DataFloat;