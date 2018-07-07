// make zip file including these files, WiFi.h (WiFiUdp included..?) and ArduinoJson.h libraries

#ifndef WiFiPayload_h
#define WiFiPayload_h

#include "Arduino.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Data.h>
//#include <nvs_flash.h> not needed anymore

#define circ_length 4096

extern bool connected;

//const char * udpAddress = "192.1.168.86";

//const int udpPort = 12345;

//class Data;

class WiFiPayload /*: WiFiClass*/ /*WiFiUDP*/ {
    public:

        //WiFiUDP& udp;

        WiFiPayload(WiFiUDP&);

        ~WiFiPayload();

        void set_device_name(String);

        const char* get_networkName();

        const char* get_networkPswd();

        int write();

        void run();

        void connectToWiFi();

        static void WiFiEvent(WiFiEvent_t event);

        void clear_data();

    private:

        char circ_buf[circ_length];

        size_t in = 0;

        size_t out = 0;
        
        //void WiFiEvent(WiFiEvent_t);

        void write_to_circ();

        int read_from_circ(); // returns 1 on success, 0 on failure.

        void heartbeat();

        const char * networkName = "bitchass jr.";
        const char * networkPswd = "watermelon";
        const char * udpAddress = "10.0.0.176";
        const int udpPort = 12345;

        // bool connected = false;
        
        WiFiUDP& udp;

        Data* data; // this must be a pointer, since the data object contains reference to a jsonObjcet, which cannot be reassigned during clear_data.
        //Insted, an entirely new Data object must be created.

        //WiFiClass& wifi;

        char mes_buf[1024]; // max size of message

        int mes_length = 0;

        //DynamicJsonBuffer json(max_length);

        String device_name = "No_Name";

        time_t time = (int)now()%5;
};

#endif