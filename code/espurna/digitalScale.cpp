#include "digitalScale.h"

// MQTT packet data
t_mqtt_packet_data g_mqtt_packet_data;
// declare for global variable
SoftwareSerial SUART( 9, 10); // TX: D1 & RX: D2

void _settingCOM()
{
    // baurate : 9600
    UART_PORT.begin(BAUND_RATE_MCU);
    DEBUG_MSG_P(PSTR("Setting baudrate of UART_PORT to 9600.\n"));
}

String _sendCMD(String command)
{
    String responce = RESPONCE_ERROR;
    DEBUG_MSG_P(PSTR("[MCU] Command request: %s\n"), command.c_str());
    UART_PORT.println(command);
    //Read responce from Scale
    while(UART_PORT.available() > 0)
    {
        responce = UART_PORT.readString();
        DEBUG_MSG_P(PSTR("[ICS449] Responce: %s\n\n"), responce.c_str());
    }
    return responce;
}

void _sendDataWithSTMode()
{
    const String topic_str = MQTT_TOPIC_WEIGHT;
    String responce = RESPONCE_ERROR;
    float weight_f = 0.0;
    while(UART_PORT.available() > 0)
    {
        responce = UART_PORT.readString();
        DEBUG_MSG_P(PSTR("[ICS449] responce: %s\n"), responce.c_str());
        if(responce.substring(START_WEIGHT_POSITION_ST, END_WEIGHT_POSITION_ST) == RECEIVE_NET_WEIGHT_VALUE)
        {
            g_mqtt_packet_data.weight_str = responce.substring(ST_START_WEIGHT_POSITION, ST_END_WEIGHT_POSITION);
            g_mqtt_packet_data.weight_str.trim();
            weight_f = atof(g_mqtt_packet_data.weight_str.c_str());
            if(weight_f <= 0)
            {
                DEBUG_MSG_P(PSTR("[MCU] Invalid weight value.\n"));
            } else
            {    
                DEBUG_MSG_P(PSTR("[MCU] Start send data\n"));
                // check for wifi/mqtt connection
                if(wifiConnected() == false) // can not connect to wifi
                {
                    // send data to lora receiver
                    Lora_SendWeight(g_mqtt_packet_data.weight_str, g_mqtt_packet_data.number_n);
                } else
                {
                    // handle by MQTT
                    mqttSend4Scale(topic_str.c_str(), g_mqtt_packet_data.weight_str.c_str());
                }
                DEBUG_MSG_P(PSTR("[MCU] Get weight: success\n"));
            }
            DEBUG_MSG_P(PSTR("[ICS449] weight value: %s kg\n"), String(weight_f).c_str());

        } else
        {
            DEBUG_MSG_P(PSTR("[MCU] Responce: %s."), responce.c_str());
        }
    }
}

void _settingSTActive()
{
    (void)_sendCMD(SEND_WEIGHT_BY_PRESSING_KEY);
    nice_delay(1);
    (void)_sendCMD(SEND_WEIGHT_BY_PRESSING_KEY);
    nice_delay(1);
    (void)_sendCMD(SEND_WEIGHT_BY_PRESSING_KEY);
    nice_delay(1);
}

int8_t _settingScale()
{
    // Currently, get the setting result but not process for failure responce
    int8_t ret = E_OK;
    // setting COM
    // -- baudrate : 9600
    // -- 8 bits
    // -- non parity
    // -- 1 stop bit
    // -- non handshake
    /* setting RS232 */
    // start configuration, connection will be suppended
    ret *= responceRS232Setting(_sendCMD(STARTS_CONFIGURATION));
    // -- baudrate : 9600
    ret *= responceRS232Setting(_sendCMD(SETTING_RS232_BAUDRATE));
    // -- 8 bits and non parity
    ret *= responceRS232Setting(_sendCMD(SETTING_RS232_BIT));
    // -- none handshake
    ret *= responceRS232Setting(_sendCMD(SETTING_RS232_HANDSHAKE));
    // -- EOL is \r\n
    ret *= responceRS232Setting(_sendCMD(SETTING_RS232_EOL));
    // end configuration, connection will be resumed
    ret *= responceRS232Setting(_sendCMD(ENDS_CONFIGURATION));
    _settingSTActive();
    
    return ret;
} 

void _checkWifiConnect()
{
    static bool isAllowed = false;
    if(isAllowed)
    {
        // can not connect to wifi
        #ifdef LORA_SENDER
        if(relayStatus(WIFI_SIGNAL_SENDER) != wifiConnected())
        {
            relayToggle(WIFI_SIGNAL_SENDER);
        }
        #endif
        #ifdef LORA_RECEIVER
        if(relayStatus(WIFI_SIGNAL_RECEIVER) != wifiConnected())
        {
            relayToggle(WIFI_SIGNAL_RECEIVER);
        }
        #endif
    } else {
        if(millis() > 6000)
            isAllowed = true;
    }
}

void _test_cmd()
{
    terminalRegisterCommand(F("TEST.LORA"), [](const terminal::CommandContext&) {
        Lora_SendWeight("25.67", g_mqtt_packet_data.number_n);
        terminalOK();
    });

    terminalRegisterCommand(F("TEST.MQTT"), [](const terminal::CommandContext&) {
        const String weight_temp = "25.67";
        mqttSend4Scale(MQTT_TOPIC_WEIGHT, weight_temp.c_str());
        terminalOK();
    });

    terminalRegisterCommand(F("TEST.M24"), [](const terminal::CommandContext&) {
        String responce = RESPONCE_ERROR;
        responce = _sendCMD("M24 2");
        DEBUG_MSG_P(PSTR("[TEST] RES: %s\n"), responce.c_str());
        terminalOK();
    });

    terminalRegisterCommand(F("TEST.M24_1"), [](const terminal::CommandContext&) {
        String responce = RESPONCE_ERROR;
        responce = _sendCMD("M24 1");
        DEBUG_MSG_P(PSTR("[TEST] RES: %s\n"), responce.c_str());
        terminalOK();
    });

    terminalRegisterCommand(F("TEST.K2"), [](const terminal::CommandContext&) {
        String responce = RESPONCE_ERROR;
        responce = _sendCMD("K 2");
        DEBUG_MSG_P(PSTR("[TEST] RES: %s\n"), responce.c_str());
        terminalOK();
    });

    terminalRegisterCommand(F("CMD.S"), [](const terminal::CommandContext&) {
        String responce = RESPONCE_ERROR;
        responce = _sendCMD("S");
        DEBUG_MSG_P(PSTR("[TEST] RES: %s"), responce.c_str());
        terminalOK();
    });

    terminalRegisterCommand(F("CMD.ST"), [](const terminal::CommandContext&) {
        String responce = RESPONCE_ERROR;
        responce = _sendCMD("ST");
        DEBUG_MSG_P(PSTR("[TEST] RES: %s\n"), responce.c_str());
        terminalOK();
    });

    terminalRegisterCommand(F("CMD.ST.1"), [](const terminal::CommandContext&) {
        String responce = RESPONCE_ERROR;
        responce = _sendCMD("ST 1");
        DEBUG_MSG_P(PSTR("[TEST] RES: %s\n"), responce.c_str());
        terminalOK();
    });
    
    terminalRegisterCommand(F("CMD.DW"), [](const terminal::CommandContext&) {
        String responce = RESPONCE_ERROR;
        responce = _sendCMD("DW");
        DEBUG_MSG_P(PSTR("[TEST] RES: %s\n"), responce.c_str());
        terminalOK();
    });

    terminalRegisterCommand(F("CMD.D"), [](const terminal::CommandContext&) {
        String responce = RESPONCE_ERROR;
        responce = _sendCMD("D \"test\"");
        DEBUG_MSG_P(PSTR("[TEST] RES: %s\n"), responce.c_str());
        terminalOK();
    });

    terminalRegisterCommand(F("CMD.T"), [](const terminal::CommandContext&) {
        String responce = RESPONCE_ERROR;
        responce = _sendCMD("T");
        DEBUG_MSG_P(PSTR("[TEST] RES: %s\n"), responce.c_str());
        terminalOK();
    });
}


void icsSetup()
{
    #ifdef LORA_SENDER
    // setting baudrate 9600 between Scale and MCU
    _settingCOM();
    // lora configuration
    LoRaSetup();
    // setting Scale ICS449
    _settingScale();
    // register loop for all functions
    espurnaRegisterLoop(_sendDataWithSTMode);
    espurnaRegisterLoop(_checkWifiConnect);
    espurnaRegisterLoop(LoRa_Receive_Callback);
    _test_cmd();
    #endif
    
    #ifdef LORA_RECEIVER
    // lora configuration
    LoRaSetup();
    // lora receiver interrupt
    espurnaRegisterLoop(_checkWifiConnect);
    //espurnaRegisterLoop(LoRa_Receive_Callback);
    #endif
}