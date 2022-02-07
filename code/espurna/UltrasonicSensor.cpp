#include "UltrasonicSensor.h"
#include "mqtt.h"
#include "utils.h"


#define LEVEL_UPDATE_INTERVAL        10


SoftwareSerial Ultra_Serial(Rxpin,Txpin);
ModbusRTU Mb_Ultrasonic;
uint8_t TX_Buffer_Data[30];
int RX_Data, RX_Data_1;
uint8_t len = 0;

uint16_t Data_Receive[32];




bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data)
{
    DEBUG_MSG_P(PSTR("[Ultrasonic] Into CbWrite\n"));
    if(event != Modbus::EX_SUCCESS)
    {
        DEBUG_MSG_P(PSTR("[Ultrasonic] Request error: 0x%02X \n"), event);
        DEBUG_MSG_P(PSTR("[Ultrasonic] Request error"));
    }
    else{
       // float flevel = Mb_Ultrasonic.Hreg(0);

       
       //RX_Data = Mb_Ultrasonic.Ireg(0);
      // RX_Data_1 = Mb_Ultrasonic.Ireg(1);
        RX_Data = Mb_Ultrasonic.Ireg(0);
        RX_Data = (RX_Data << 16) | Mb_Ultrasonic.Ireg(1);

        union  
        {
            unsigned char Rx_REG[4];
            float F_data;
        }Data_F;
       
        Data_F.Rx_REG[3] = char(RX_Data >> 24);  
        Data_F.Rx_REG[2] = char((RX_Data>>16)&0x00FF);
        Data_F.Rx_REG[1] = char((RX_Data>>8)&0x0000FF);
        Data_F.Rx_REG[0] = char(RX_Data & 0x000000FF);


        DEBUG_MSG_P(PSTR("RX_Data[3]: %s\n"),String(Data_F.Rx_REG[3]).c_str());
        DEBUG_MSG_P(PSTR("RX_Data[2]: %s\n"),String(Data_F.Rx_REG[2]).c_str());
        DEBUG_MSG_P(PSTR("RX_Data[1]: %s\n"),String(Data_F.Rx_REG[1]).c_str());
        DEBUG_MSG_P(PSTR("RX_Data[0]: %s\n"),String(Data_F.Rx_REG[0]).c_str()); 
        DEBUG_MSG_P(PSTR("RX_Data: %d\n"),RX_Data);
        DEBUG_MSG_P(PSTR("RX_Data_1: %d\n"),RX_Data_1);
        DEBUG_MSG_P(PSTR("RX_Data: %s\n"),String(Data_F.F_data).c_str());
        
        // mqttSend("Level",String(Data_F.F_data).c_str());
        // mqttSend("Level","sdsdsds");
        
    }

    return true;
}

void UltrasonicSensor_Loop()
{   
    static unsigned long lastUpdate = 0;

    if(millis() > lastUpdate + (LEVEL_UPDATE_INTERVAL * 1000))
    {

        lastUpdate = millis();
        DEBUG_MSG_P(PSTR("[Ultrasonic] Request data test\n"));
        if(!Mb_Ultrasonic.slave())
        {
            DEBUG_MSG_P(PSTR("[Ultrasonic] Request data\n"));
           // nice_delay(10);
            Mb_Ultrasonic.readIreg(0x01,0,nullptr,2,cbWrite);
        }

    }

    if(Mb_Ultrasonic.slave())
    {
        Mb_Ultrasonic.task();
        yield();
    }

   // nice_delay(1000);


}
 

void UltrasonicSensor_Setup()
{
    Ultra_Serial.begin(9600);
    Mb_Ultrasonic.begin(&Ultra_Serial);
    Mb_Ultrasonic.addIreg(0,0,2);
    //Mb_Ultrasonic.setBaudrate(9600);
    Mb_Ultrasonic.master();
    espurnaRegisterLoop(UltrasonicSensor_Loop);
}


