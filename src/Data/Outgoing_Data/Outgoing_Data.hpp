
#ifndef Outgoing_Data_hpp
#define Outgoing_Data_hpp

#include "Arduino.h"
#include <ArduinoJson.h>
#include "../Data.hpp"

extern int max_length;

class WiFiPayload;

class Outgoing_Data : public Data{

    Outgoing_Data();

    ~Outgoing_Data();

    // adds key (key_) value (t) pair to DATAroot
    template<typename T> int add_data(const char* key_, T t){ // relies on implicit casts, overwrites if index already exists
        if(Node* it = find_custom("data")){
            if(it->var.as<JsonObject&>()[key_] = t){
                return 0; // success
            }
            return -4; // allocation failed because no space left in buffer
        }
        return -5; // unintended internal error
    }

    // adds object named "key_" to DATAroot
    int add_object(const char* key_); // wrapper around create_object

    // adds array named "key_" to DATAroot
    int add_array(const char* key_); // wrapper around create_array

    // nests the new array named "arr_key" inside object named "outer"
    int create_nested_array(const char* outer, const char* arr_key);

    // nests the new object named "obj_key" inside object named "outer"
    int create_nested_object(const char* outer, const char* obj_key);

    // adds field named "index" with value t to object named key_
    template<typename T> int add_to_object(const char* key_, const char* index, T t){
        if(Node* it = find_custom(key_)){
            if(it->get_field().success()){
                if(it->get_field().is<JsonObject>()){
                    if(it->get_field().as<JsonObject&>().set(index, t)){
                        return 0; // success
                    }
                    return -4; // JsonObject::set() returns false if JsonBuffer full
                }
                return -3; // key_ is an array
            }
            return -5; // internal error
        }
        return -1; // key not found
    }

    // adds value t to position index of array named key_
    template<typename T> int add_to_array(const char* key_, size_t index, T t){ // add data to either custom_array or custom_object
        if(Node* it = find_custom(key_)){
            if(it->get_field().success()){
                if(it->get_field().is<JsonArray>()){
                    if(index >= it->capacity){
                        while(it->capacity <= index){
                            it->get_field().as<JsonArray&>().add(false); // what should be added to the empty space?
                            it->capacity++;
                        }                            
                    }
                    if(it->get_field().as<JsonArray&>().set(index, t)){
                        return 0;
                    }
                    return -4; // JsonArray::set() returns false if JsonBuffer full
                }
                return -3; // key_ is an object
            }
            return -5; // unintended internal error
        }
        return -1; // key not found
    }

    // helper function that generates an object named "key_" nested inside object custom
    int create_object(const char* key_, JsonVariant& custom);

    // helper function that generates an array named "key_" nested inside object custom
    int create_array(const char* key_, JsonVariant& custom);

    // clears hash table, resets jsonBuffer, creates new DATAroot object with a "data" nested object
    void clear();

    friend class WiFiPayload;

};

#endif

        