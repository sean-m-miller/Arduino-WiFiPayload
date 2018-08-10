#ifndef WiFiPayload_h
#define WiFiPayload_h

#include "Arduino.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <FastCRC.h> // https://github.com/FrankBoesing/FastCRC

#include "Data/Outgoing_Data/Outgoing_Data.hpp"
#include "Data/Incoming_Data/Incoming_Data.hpp"
#include "CircBuff/OutBuff/OutBuff.hpp"
#include "CircBuff/InBuff/InBuff.hpp"
#include <Values.hpp> // for values namespace

/* ERROR CODES for outgoing message functions:
     0 Success
    -1 key_ or index does not exist
    -2 key_ or index already exists
    -3 JsonObject function called on JsonArray, or vice versa
    -4 Outgoing message is full 
    -5 Unintended internal error - please report */

/* ERROR CODES for incoming message functions:
     0 success
    -1 key or index does not exist
    -2 object or array does not exist
    -3 JsonObject function called on JsonArray, or vice versa
    -4 No message from DDS available to be parsed
    -5 Unintended internal error - please report */

class WiFiPayload {
    
    public:

        // Constructor that takes in ssid (network name), network password, ip of rpi server, port that rpi server is listening on
        WiFiPayload(const char* ssid, const char* pswd, const char* udpAdd, const int udpP);

        // Destructor that does nothing 
        ~WiFiPayload();

        // returns the number of bytes being used (plus dead space) by the buffer storing the message to DDS
        size_t out_msg_size();

        // returns the free bytes left in the outgoing message
        size_t out_msg_space_left();

        // returns the size of the message (in bytes) received from DDS
        size_t in_msg_size();

        // sets device name. Must be unique from other external payload devices (if any)
        int set_device_name(const char* name);

        // set myWiFi to the externed WiFi object and connect to wifi. returns 0 on success, -1 if device_name has not been set
        int begin(); 

        // getter function to see if WiFiPayload is connected to Trident WiFi
        bool is_connected();
    
        // adds a key (key_) value (t) pair to the outgoing data message. relies on implicit casts, overwrites if key_ already exists. 
        // returns 0 on success, -4 if outgoing message is full 
        template<typename T> int add_data(const char* key_, T t){ // 
            return out_data.add_data(key_, t);
        };

        // adds an object named key_ to the outgoing message. 
        // returns 0 on success, -2 if an object named key_ already exists, -4 if outgoing message is full
        int add_object(const char* key_);

        // adds an array named key_ to the outgoing message. 
        // returns 0 on success, -2 if an array named key_ already exists, -4 if outgoing message is full
        int add_array(const char* key_);

        // creates an object called obj_key nested inside the object called outer_key
        // returns 0 on success, -1 if object named outer_key not found, -2 if object called obj_key already exists,
        // returns -3 if outer_key is an array (instead of an object), -4 if outgoing message is full 
        int create_nested_object(const char* outer_key, const char* obj_key);
        
        // creates an array called arr_key nested inside the object called outer_key
        // returns 0 on success, -1 if object named outer_key not found, -2 if object called obj_key already exists,
        // returns -3 if outer_key is an array (instead of an object), -4 if outgoing message is full
        int create_nested_array(const char* outer_key, const char* arr_key);

        // adds the key (index) value (t) pair to the object named key_. Relies on implicit casts and overwrites if index already exists
        // returns 0 on success, -1 if object named key_ not found, -3 if key_ is an array,
        // returns -4 if outgoing message is full
        template<typename T> int add_to_object(const char* key_, const char* index, T t){
            return out_data.add_to_object(key_, index, t);
        }

        // adds the value t to array named key_ at position index. The array grows dynamically, any indices without values at firstdefault to false.
        // Relies on implicit casts and overwrites if index already exists
        // returns 0 on success, -1 if object named key_ not found, -3 if key_ is an object,
        // returns -4 if outgoing message is full
        template<typename T> int add_to_array(const char* key_, size_t index, T t){
            return out_data.add_to_array(key_, index, t);
        }

        // sends outgoing message to DDS via write_buf and clears the outgoing message. 
        // Returns -1 and does not write (still clears outgoing message) if device_name was never set.
        int write();

        // clears the previous message from DDS and retrieves the next message from DDS via read_buf.
        // returns 0 on success, -1 if no messages were available, -2 if message available was corrupted (crc did not match)
        int read();

        // parses the field named "key_" and captures the result in destination.
        // Returns 0 on success, -1 if key_ does not name a field.
        // Returns -4 if no message available to be parsed.
        template<typename T> int parse_data(const char* key_, T& destination){
            if(ready){
                return in_data.parse_data(key_, destination);
            }
            return -4;
        }

        // parses the field named "index" of the object (can be nested) named "key_" and captures the result in destination
        // returns 0 on success, -1 if index does not name a field, -2 if key_ does not exist.
        // returns -3 if key_ names an array, and returns -4 if no message available to be parsed.
        template<typename T> int parse_object(const char* key_, const char* index, T& destination){
            if(ready){
                return in_data.parse_object(key_, index, destination);
            }
            return -4;
        }

        // finds the position index in array named "key_" and captures the value in destination.
        // Returns 0 on success, -1 if index does not exist, -2 if key_ does not exist.
        // Returns -3 if key_ names an object, and returns -4 if no message available to be parsed.
        template<typename T> int parse_array(const char* key_, size_t index, T& destination){
            if(ready){
                return in_data.parse_array(key_, index, destination);
            }
            return -4;
        }

        // parses the field named "key_" and copies the cstring value into destination.
        // Returns 0 on success, -1 if key_ does not name a field.
        // Returns -4 if no message available to be parsed.
        int parse_data_cstring(const char* key_, char* destination);

        // parses the field named "index" in the object named "key_" and copies the value into destination.
        // Returns 0 on success, returns -1 if no field named "index".
        // Returns -2 if no object named "key_", returns -3 if "key_" names an array
        // Returns -4 if no message available to be parsed.
        int parse_object_cstring(const char* key_, const char* index, char* destination);

        // parses the field at position index of array named "key_" and copies the value into destination.
        // Returns 0 on success, -1 if index does not exist, -2 if key_ does not exist.
        // Returns -3 if key_ names an object and -4 if no message available to be parsed.
        int parse_array_cstring(const char* key_, size_t index, char* destination);

        // performs various updates and signals to Trident that this device is connected and active
        void heartbeat();

        // copies the outgoing message to destination
        void print_out_data_to(char* destination);

        // copies the incoming message to destination
        void print_in_data_to(char* destination);

    private:

        // connects WiFiPayload object to WiFi
        void connectToWiFi();

        // wrapper object that stores outgoing message
        Outgoing_Data out_data;

        // wrapper object that stores incoming message
        Incoming_Data in_data;

        // flag set when WiFiPayload Object is connected to WiFi
        volatile bool connected = false;

        // flag set when an incoming message has been successfully received and is ready to be parsed.
        volatile bool ready = false;

        // circular buffer used to store outgoing messages before they get sent
        OutBuff write_buf;

        // circular buffer used to store incoming messages before they get read
        InBuff read_buf;

        // called in heartbeat(), checks if udp packet available, and copies into read_buf
        int receive_into_read_buf();

        // ssid
        char networkName[values::NAME_SIZE];

        // pswd
        char networkPswd[values::NAME_SIZE];

        // ip address of Trident
        char udpAddress[values::NAME_SIZE];

        // port Trident is lsitening on
        unsigned int udpPort;

        // class pointer to extern WiFiClass object
        WiFiClass* myWiFi = NULL;
        
        // udp object
        WiFiUDP udp;

        // buffer used temporarily for outgoing messages. + 4 to leave space for crc
        char write_mes_buf[values::MESSAGE_SIZE + 4];

        // buffer used temporarily for incoming messages. + 4 to leave space for crc
        char read_mes_buf[values::MESSAGE_SIZE + 4];

        // name for WiFiPayload object. Used as unique identifier for the Trident server connections map
        char device_name[values::NAME_SIZE] = "No_Name";
};



#endif
