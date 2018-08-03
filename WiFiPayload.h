#ifndef WiFiPayload_h
#define WiFiPayload_h

#include "Arduino.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Outgoing_Data.h>
#include <Incoming_Data.h>
#include <OutBuff.h>
#include <InBuff.h>
#include <Values.h> // for values namespace

#include <FastCRC.h> // https://github.com/FrankBoesing/FastCRC

extern bool connected;

/* ERROR CODES:
     0 Success
    -1 key_ or index does not exist
    -2 key_ or index already exists
    -3 JsonObject function called on JsonArray, or vice versa
    -4 Message is full */

class WiFiPayload {
    
    public:

         /**
         * @brief Constructor that takes in 
         * 
         * @param ssid (network name), network password, ip of rpi server, port that rpi server is listening on.
         */
        WiFiPayload(const char* ssid, const char* pswd, const char* udpAdd, const int udpP);

        /**
         * @brief Destructor that does nothing 
         */
        ~WiFiPayload();

        /**
         * @brief returns the number of bytes being used (plus dead space) by the buffer storing the message to DDS. 
         */
        size_t out_msg_size();

        /**
         * @brief returns the size of the message (in bytes) received from DDS.
         */
        size_t in_msg_size();

        void begin(); // set myWiFi to the externed WiFi object.

        bool is_connected();
        /**
         * @brief sets the name of this device. This function must be called before messages can be written to DDS. 
         * 
         * @param ssid (network name), network password, ip of rpi server, port that rpi server is listening on.
         */
        int set_device_name(const char* name);

        template<typename T> size_t add_data(const char* key_, T t){ // relies on implicit casts, overwrites if index already exists
            return out_data.add_data(key_, t);
        };

        int add_object(const char* key_); // wrapper around create_object

        int add_array(const char* key_); // wrapper around create_array

        int create_nested_array(const char* key_, const char* arr_key);

        int create_nested_object(const char* key_, const char* obj_key);

        //custom_object overload
        template<typename T> int add_to_object(const char* key_, const char* index, T t){ // add data to either custom_array or custom_object
            return out_data.add_to_object(key_, index, t);
        }

        //custom_array overload
        template<typename T> int add_to_array(const char* key_, size_t index, T t){ // add data to either custom_array or custom_object
            return out_data.add_to_array(key_, index, t); //key not found or function was called on object
        }

        int write();

        template<typename T> int parse_data(const char* key_, T& destination){
            return in_data.parse_data(key_, destination);
        }

        template<typename T> int parse_object(const char* key_, const char* index, T& destination){
            return in_data.parse_object(key_, index, destination);
        }

        template<typename T> int parse_array(const char* key_, size_t index, T& destination){
            return in_data.parse_array(key_, index, destination);
        }

        int parse_data_string(const char* key_, char* destination); // overload for basic field

        int parse_object_string(const char* key_, const char* index, char* destination); // overload for custom object

        int parse_array_string(const char* key_, size_t index, char* destination); // overload for custom array

        int read();

        void connectToWiFi();

        void heartbeat();

        void print_out_data_to(char* destination, size_t size);

        void print_in_data_to(char* destination, size_t size);

    private:

        Outgoing_Data out_data;

        Incoming_Data in_data;

        volatile bool connected;

        //void WiFiEvent(WiFiPayload* self, WiFiEvent_t event); // implied static as well

        OutBuff write_buf;

        InBuff read_buf;

        int read_into_buf();

        char networkName[30];

        char networkPswd[30];

        char udpAddress[30];

        unsigned int udpPort;

        WiFiClass* myWiFi = NULL; // such a shame... The extern WiFiClass object "WiFi" makes this necessary to wrap WiFiEvent in the WiFiPayload class
        
        WiFiUDP udp;

        char write_mes_buf[values::MESSAGE_SIZE]; // max size of message

        char read_mes_buf[values::MESSAGE_SIZE];

        int mes_length = 0;

        char device_name[25] = "No_Name";

        int time = -1; // heartbeat immediately (will always pass if statement within 3 seconds)

        class HashPrint : public Print {
            
            public:

                HashPrint();

                virtual size_t write(uint8_t c);

                uint32_t hash() const;

            private:

                FastCRC32 _hasher;

                uint32_t _hash;
        };
};



#endif