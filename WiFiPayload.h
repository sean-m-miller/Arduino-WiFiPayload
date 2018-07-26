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

#define MESSAGE_SIZE 1024

class WiFiPayload {
    
    public:

        WiFiPayload();

        ~WiFiPayload();

        void set_device_name(const char* name);

        template<typename T> size_t add_data(const char* key_, T t){ // relies on implicit casts, overwrites if index already exists
            return out_data.add_data(key_, t);
        };

        size_t add_object(const char* key_); // wrapper around create_object

        size_t add_array(const char* key_); // wrapper around create_array

        size_t create_nested_array(const char* key_, const char* arr_key);

        size_t create_nested_object(const char* key_, const char* obj_key);

        //custom_object overload
        template<typename T> int add_to_object(const char* key_, const char* index, T t){ // add data to either custom_array or custom_object
            return out_data.add_to_object(key_, index, t);
        }

        //custom_array overload
        template<typename T> int add_to_array(const char* key_, size_t index, T t){ // add data to either custom_array or custom_object
            return out_data.add_to_array(key_, index, t); //key not found or function was called on object
        }

        size_t write();

        template<typename T> size_t parse_data(const char* key_, T& destination){
            return in_data.parse_data(key_, destination);
        }

        template<typename T> size_t parse_object(const char* key_, const char* index, T& destination){
            return in_data.parse_object(key_, index, destination);
        }

        template<typename T> size_t parse_array(const char* key_, size_t index, T& destination){
            return in_data.parse_array(key_, index, destination);
        }

        size_t parse_data_string(const char* key_, char* destination); // overload for basic field

        size_t parse_object_string(const char* key_, const char* index, char* destination); // overload for custom object

        size_t parse_array_string(const char* key_, size_t index, char* destination); // overload for custom array

        size_t read();

        void connectToWiFi();

        void heartbeat();

        void print_out_data_to(char* destination, size_t size);

        void print_in_data_to(char* destination, size_t size);

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

        char write_mes_buf[MESSAGE_SIZE]; // max size of message

        char read_mes_buf[MESSAGE_SIZE];

        int mes_length = 0;

        char device_name[25] = "No_Name";

        int time = -1; // heartbeat immediately (will always pass if statement within 3 seconds)
};



#endif