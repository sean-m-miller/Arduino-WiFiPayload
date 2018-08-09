#ifndef Data_hpp
#define Data_hpp

#include "Arduino.h"
#include <ArduinoJson.h>
#include "../Values.hpp" // for values namespace

class Data{

    public:

        size_t msg_size();

        void print_hash_table();

    protected:

        StaticJsonBuffer<1024> jsonBuffer; 

        JsonVariant DATAroot;

        class Node{

            public:

                Node(const char* key_, JsonVariant var);

                Node(){};

                virtual ~Node(){
                    next = NULL;
                };

                char key[30]; //arbitrary max length for key

                Node* next = NULL;

                JsonVariant var;

                JsonVariant& get_field();

                size_t capacity = 0;
        };

        class Custom_Object : public Node {

            public:

                Custom_Object(const char* key_, JsonVariant& obj); // used by outgoing Data

                Custom_Object(const char* key_, JsonObject& obj); // used by incoming data

                virtual ~Custom_Object(){};

                JsonObject& obj;

        };

        class Custom_Array : public Node {

            public:

                Custom_Array(const char* key_, JsonVariant& obj);

                Custom_Array(const char* key_, JsonArray& arr);

                virtual ~Custom_Array(){};
                
                JsonArray& arr;

        };

        Node* find_custom(const char* key_);

        Node* obj_map[100];

        int hash(const char* key_);

        void custom_deleter();

        void deleter_helper(Node**);

        virtual void clear();

    friend class WiFiPayload;

};

#endif