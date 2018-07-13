#ifndef WiFiPayload_h
#define WiFiPayload_h

#include "Arduino.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Data.h>
#include <CircBuff.h>

extern bool connected;

class WiFiPayload {
    
    public:

        WiFiPayload();

        ~WiFiPayload();

        void set_device_name(String);

        const char* get_networkName();

        const char* get_networkPswd();

        template <typename W> void add_data(const char* key, W w){
            data->add(key, w);
        }

        void create_custom_object(const char* key);

        template <typename C> int add_to_custom_object(const char* key, const char* index, C c){
            data->add_to_custom_object(key, index, c);
        }

        void create_array(const char* key);

        void remove(const char* key);

        void write();

        size_t get_capacity();

        void connectToWiFi();

        static void WiFiEvent(WiFiEvent_t event);

        void heartbeat();

        Data* data; // this must be a pointer, since the data object contains reference to a jsonObjet, which cannot be reassigned during clear_data.
        //Insted, an entirely new Data object must be created.

        CircBuff buf;

    private:

        
        
        //void WiFiEvent(WiFiEvent_t);

        void clear_data();

        void write_to_buf();

        int read_from_buf(); // returns 1 on success, 0 on failure.

        const char * networkName = "OpenROV";
        const char * networkPswd = "bilgepump";
        const char * udpAddress = "192.168.1.86";
        const int udpPort = 12345;
        
        WiFiUDP udp;

        char mes_buf[1024]; // max size of message

        int mes_length = 0;

        String device_name = "No_Name";

        int time = -1; // heartbeat immediately (will always pass if statement within 3 seconds)
};

// extern WiFiClass WiFi; // ensures construction before other extern WiFi

//extern WiFiUDP udp;

//extern CircBuff buf;

#endif