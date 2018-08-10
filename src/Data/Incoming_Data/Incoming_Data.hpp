#ifndef Incoming_Data_hpp
#define Incoming_Data_hpp

#include "Arduino.h"
#include <ArduinoJson.h>
#include "../Data.hpp"

class Incoming_Data : public Data {

    Incoming_Data();

    // recursively constructs obj_map out of a parsed Json message
    void create_obj_map(JsonVariant obj);

    // used in create_obj_map()
    int create_array(const char* key_, JsonArray& custom);

    // used in create_obj_map()
    int create_object(const char* key_, JsonObject& custom);

    // parses the "data" object for field named "index", and captures the value in destination
    template<typename T> int parse_data(const char* index, T& destination){
        if(Node* it = find_custom("data")){
            if(it->var.is<JsonObject>()){
                if(it->get_field().as<JsonObject&>().containsKey(index)){
                    destination = it->get_field()[index];
                    return 0; // success
                }
                return -1; // key error
            }
            return -5; // unintended internal error
        }
        return -5; // unintended internal error
    }

    // parses the field named "index" in the object named "key_", and captures the value in destination
    template<typename T> int parse_object(const char* key_, const char* index, T& destination){
        if(Node* it = find_custom(key_)){
            if(it->var.is<JsonObject>()){
                if(it->get_field().as<JsonObject&>().containsKey(index)){
                    destination = it->get_field()[index];
                    return 0; // success
                }
                return -1; // index does not exist
            }
            return -3; // key_ names an array
        }
        return -2; // no custom named key_
    }

    // captures the value stored at position index of array named "key_" in destination
    template<typename T> int parse_array(const char* key_, size_t index, T& destination){
        if(Node* it = find_custom(key_)){
            if(it->var.is<JsonArray>()){
                if(index < it->capacity){
                    destination = it->get_field()[index];
                    return 0; // success
                }
                return -1; // index does not exist
            }
            return -3; // key_ names an object
        }
        return -2; // no custom named key_
    }

    // copies cstring stored at "key_" into destination
    int parse_data_cstring(const char* key_, char* destination);

    // copies string stored at field "index" of object named "key_" into destination
    int parse_object_cstring(const char* key_, const char* index, char* destination);

    // copies cstring stored at position index of array named "key_" into destination
    int parse_array_cstring(const char* key_, size_t index, char* destination);

    // clears hash table and resets jsonBuffer and DATAroot
    void clear();

    // friendship is not inherited, so must be redeclared here
    friend class WiFiPayload;
};

#endif