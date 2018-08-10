#ifndef Data_hpp
#define Data_hpp

#include "Arduino.h"
#include <ArduinoJson.h>
#include "../Values.hpp" // for values namespace

// wrapper around the ArduinoJson library
class Data{

    protected:

        // returns amount of jsonBuffer being used
        size_t msg_size();

        // used for testing and debugging
        void print_hash_table();

        // Buffer for storing Json objects
        StaticJsonBuffer<1024> jsonBuffer; 

        // "root" of the data object. 
        JsonVariant DATAroot;

        // node class for the hash table storing all ArduinoJson JsonObjects and JsonArrays.
        // Without the hash table, modifying any custom object requires searching through all fields of all customs starting with DATAroot (O(N)).
        // Hash table sacrafices some heap memory for O(1).
        class Node{

            public:

                Node(const char* key_, JsonVariant var);

                Node(){};

                virtual ~Node(){
                    next = NULL;
                };

                char key[values::NAME_SIZE];

                Node* next = NULL;

                // container for JsonObject& or JsonArray&
                JsonVariant var;

                // getter function in case in the future this becomes more modular. At this point, calling field var is identical
                JsonVariant& get_field();

                // this field is only used by the Custom_Array nodes.
                size_t capacity = 0;
        };

        class Custom_Object : public Node {

            public:

                // Outgoing_Data overload
                Custom_Object(const char* key_, JsonVariant& obj); 

                // Incoming_Data overload
                Custom_Object(const char* key_, JsonObject& obj);

                virtual ~Custom_Object(){};

                JsonObject& obj;
        };

        class Custom_Array : public Node {

            public:

                // Outgoing_Data overload
                Custom_Array(const char* key_, JsonVariant& obj);

                // Incoming_Data Overload
                Custom_Array(const char* key_, JsonArray& arr);

                virtual ~Custom_Array(){};
                
                JsonArray& arr;
        };

        // returns pointer to the Custom_Array or Custom_Object named "key_"
        Node* find_custom(const char* key_);

        // hash table of all custom_objects
        Node* obj_map[100];

        // custom hash function
        int hash(const char* key_);

        // clears hash table
        void custom_deleter();

        // used by custom_deleter(), clears a single index of the hash table
        void deleter_helper(Node**);

        // clears hash table, and resets jsonBuffer and DATAroot
        virtual void clear();

    // bad practice but no way user could get access to this class
    friend class WiFiPayload;

};

#endif