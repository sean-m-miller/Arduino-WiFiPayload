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

class Data; // used for Node* in parse()

class Node; // used for Node* in parse()

class WiFiPayload {
    
    public:

        WiFiPayload();

        ~WiFiPayload();

        void set_device_name(String);

        Outgoing_Data& get_out_msg();

        Incoming_Data& get_in_msg();

        size_t write();

        size_t read();

        template<typename T> size_t parse(const char* key_, T& destination){
            if(ready){ // a message has been read and in_data has it parsed.
                destination = in_data.DATAroot[key_];
                return 1; // success
            }
            return 0;
        }

        template<typename T> size_t parse(const char* key_, const char* index, T& destination){
            if(ready){ // a message has been read and in_data has it parsed.
                Node* it = in_data.find_custom(key_);
                if(it->var.is<JsonObject>()){
                    destination = it->obj[index];
                    return 1; // success
                }
            }
            return 0;
        }

        template<typename T> size_t parse(const char* key_, size_t index, T& destination){
            if(ready){ // a message has been read and in_data has it parsed.
                Node* it = in_data.find_custom(key_);
                if(it->var.is<JsonArray>()){
                    destination = it->obj[index];
                    return 1; // success
                }
            }
            return 0;
        }

        size_t parse_string(const char* key_, char* destination); // overload for basic field

        size_t parse_string(const char* key_, const char* index, char* destination); // overload for custom object

        size_t parse_string(const char* key_, size_t index, char* destination); // overload for custom array

        //size_t write_in_msg();

        void connectToWiFi();

        void heartbeat();

    private:

        Outgoing_Data out_data;

        Incoming_Data in_data;

        bool ready = false;

        static void WiFiEvent(WiFiEvent_t event);

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