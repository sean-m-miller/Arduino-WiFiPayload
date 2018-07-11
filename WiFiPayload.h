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

        void create_custom_object(const char*);

        template <typename W> void add_data(const char* key, W w){
            data->add(key, w);
        }

        template <typename C> int add_to_custom_object(const char* key, const char* index, C c){
            data->add_to_custom_object(key, index, c);
        }

        void remove(const char* key);

        void write();

        void run();

        void connectToWiFi();

        static void WiFiEvent(WiFiEvent_t event);

        void heartbeat();

        Data* data; // this must be a pointer, since the data object contains reference to a jsonObjet, which cannot be reassigned during clear_data.
        //Insted, an entirely new Data object must be created.

    private:

        char circ_buf[circ_length];

        size_t in = 0;

        size_t out = 0;
        
        //void WiFiEvent(WiFiEvent_t);

        void clear_data();

        void write_to_circ();

        int read_from_circ(); // returns 1 on success, 0 on failure.

        const char * networkName = "OpenROV";
        const char * networkPswd = "bilgepump";
        const char * udpAddress = "192.168.1.86";
        const int udpPort = 12345;

        // bool connected = false;
        
        WiFiUDP& udp;

        char mes_buf[1024]; // max size of message

        int mes_length = 0;

        //DynamicJsonBuffer json(max_length);

        String device_name = "No_Name";

        int time = -1; // heartbeat immediately (will always pass if statement within 3 seconds)
};

#endif