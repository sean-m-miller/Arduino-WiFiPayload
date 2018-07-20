#ifndef WiFiPayload_h
#define WiFiPayload_h

#include "Arduino.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Outgoing_Data.h>
#include <Incoming_Data.h>
#include <CircBuff.h>

extern bool connected;

class WiFiPayload {
    
    public:

        WiFiPayload();

        ~WiFiPayload();

        void set_device_name(String);

        Outgoing_Data& get_out_msg();

        Incoming_Data& get_in_msg();

        size_t write_out_msg();

        size_t read();

        size_t write_in_msg();

        void connectToWiFi();

        void heartbeat();

    private:

        bool ready = false;

        static void WiFiEvent(WiFiEvent_t event);

        Outgoing_Data out_data;

        Incoming_Data in_data;

        CircBuff write_buf;

        CircBuff read_buf;

        int read_into_buf();

        const char* networkName = "OpenROV";

        const char* networkPswd = "bilgepump";

        const char * udpAddress = "192.168.1.86";

        const unsigned int udpPort = 12345;
        
        WiFiUDP udp;

        char write_mes_buf[1024]; // max size of message

        char read_mes_buf[1024];

        int mes_length = 0;

        String device_name = "No_Name";

        int time = -1; // heartbeat immediately (will always pass if statement within 3 seconds)
};

#endif