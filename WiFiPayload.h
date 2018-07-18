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

        const char* get_networkName();

        const char* get_networkPswd();

        Outgoing_Data& get_out_msg();

        Incoming_Data& get_in_msg();

        // template <typename W> void add_data_out_msg(const char* key_, W w){
        //     out_data.add(key_, w);
        // }

        // template <typename W> void add_data_in_msg(const char* key_, W w){
        //     in_data.add(key_, w);
        // }

        // void create_object_out_msg(const char* key_);

        // void create_object_in_msg(const char* key_);

        // void create_array_out_msg(const char* key_);

        // void create_array_in_msg(const char* key_);

        // template <typename C> int add_to_object_out_msg(const char* key_, const char* index, C c){
        //     out_data.add_to_object(key_, index, c);
        // }

        // template <typename C> int add_to_object_in_msg(const char* key_, const char* index, C c){
        //     in_data.add_to_object(key_, index, c);
        // }

        // template <typename C> int add_to_array_out_msg(const char* key_, size_t index, C c){
        //     out_data.add_to_array(key_, index, c);
        // }

        // template <typename C> int add_to_array_in_msg(const char* key_, size_t index, C c){
        //     in_data.add_to_array(key_, index, c);
        // }

        // size_t create_nested_array_out_msg(const char* key_, const char* arr_key);

        // size_t create_nested_array_in_msg(const char* key_, const char* arr_key);

        // size_t create_nested_object_out_msg(const char* key_, const char* obj_key);

        // size_t create_nested_object_in_msg(const char* key_, const char* obj_key);

        size_t write_out_msg();

        size_t read();

        size_t write_in_msg();

        // size_t get_capacity_out_msg();

        // size_t get_capacity_in_msg();

        void connectToWiFi();

        void heartbeat();

    private:

        static void WiFiEvent(WiFiEvent_t event);

        Outgoing_Data out_data;

        Incoming_Data in_data;

        CircBuff write_buf;

        CircBuff read_buf;

        void clear_out_data();

        void clear_in_data();

        void write_to_buf();

        int read_from_buf(); // returns 1 on success, 0 on failure.

        int read_into_buf();

        const char * networkName = "bitchass jr.";
        const char * networkPswd = "watermelon";
        const char * udpAddress = "10.0.0.176";
        const unsigned int udpPort = 12345;
        
        WiFiUDP udp;

        char write_mes_buf[1024]; // max size of message

        char read_mes_buf[1024];

        int mes_length = 0;

        String device_name = "No_Name";

        int time = -1; // heartbeat immediately (will always pass if statement within 3 seconds)
};

#endif