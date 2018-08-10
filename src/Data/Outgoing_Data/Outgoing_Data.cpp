#include "Arduino.h"
#include "Outgoing_Data.hpp"

Outgoing_Data::Outgoing_Data(){

    // ensure all map pointers are set to NULL
    for(size_t i = 0; i < 100; i++){
        obj_map[i] = NULL; 
    }
    DATAroot = jsonBuffer.createObject();

    if(!DATAroot.success()){
        Serial.println("Outgoing_constructor failed, JsonObject not created out of jsonBuffer");
    }

    // ready for user to add data
    create_object("data", DATAroot);
}

Outgoing_Data::~Outgoing_Data(){
    custom_deleter();
}

int Outgoing_Data::create_object(const char* key_, JsonVariant& custom){
    if(!custom.is<JsonObject>() || !custom.success()){
         // "data" created incorrectly, or invalid custom in hash table not removed.
        return -5;
    }
    int index = hash(key_);

    // hash table collision
    if(Node* it = obj_map[index]){ 
        while(it->next!=NULL){
            if(!strcmp(it->key, key_)){
                return -2; // key error that identifier already exists
            }
            it = it->next;
        }

        //check tail
        if(!strcmp(it->key, key_)){ 
            return -2;
        }

        // set to end of list at that index
        it->next = new Custom_Object(key_, custom); 
    }
    else{

        // first object mapped to that index
        obj_map[index] = new Custom_Object(key_, custom); 
    }

    // allocation failed from not enough space in buffer. Remove from obj_map
    Node* it2 = find_custom(key_);
    if(!it2->get_field().success()){
        Node* it3 = obj_map[index];
        if(it3 == it2){
            delete it3;
            obj_map[index] = NULL;
            return -4; // jsonBuffer full
        }
        while(it3->next != it2){
            it3 = it3->next;
        }
        delete it2;
        it3->next = NULL;
        return -4; // jsonBuffer full
    }
    return 0; // success
}

int Outgoing_Data::create_array(const char* key_, JsonVariant& custom){
    if(!custom.is<JsonObject>() || !custom.success()){
        
        // "data" created incorrectly, or object in hash table not removed. 
        return -5; 
    }
    int index = hash(key_);

    // hash table collision
    if(Node* it = obj_map[index]){ 
        while(it->next!=NULL){
            if(!strcmp(it->key, key_)){
                return -2; //key error that identifier already exists
            }
            it = it->next;
        }

        // check tail
        if(!strcmp(it->key, key_)){ 
            return -2;
        }

        // set to end of list at that index
        it->next = new Custom_Array(key_, custom); 
    }
    else{

        // first object at that index
        obj_map[index] = new Custom_Array(key_, custom); 
    }

    // allocation failed from not enough space in buffer. Remove from obj_map
    Node* it2 = find_custom(key_);
    if(!it2->get_field().success()){
        Node* it3 = obj_map[index];
        if(it3 == it2){
            delete it3;
            obj_map[index] = NULL;
            return -4; // jsonBuffer full
        }
        while(it3->next != it2){
            it3 = it3->next;
        }
        delete it2;
        it3->next = NULL;
        return -4; // jsonBuffer full
    }
    return 0; // success
}

int Outgoing_Data::add_array(const char* key_){
    return create_array(key_, find_custom("data")->get_field());
}

int Outgoing_Data::add_object(const char* key_){
    return create_object(key_, find_custom("data")->get_field());
}

int Outgoing_Data::create_nested_array(const char* key_, const char* arr_key){
    Node* object = find_custom(key_);
    if(object){
        if(object->get_field().is<JsonObject>()){
            return create_array(arr_key, object->get_field());
        }
        return -3; // arrays cannot nest arrays
    }
    return -1; // key does not exist
}

int Outgoing_Data::create_nested_object(const char* key_, const char* obj_key){
    Node* object = find_custom(key_);
    if(object){
        if(object->get_field().is<JsonObject>()){
            return create_object(obj_key, object->get_field());
        }
        return -3; // arrays cannot nest objects
    }
    return -1; // key_ does not exist
}

void Outgoing_Data::clear(){

    // reset hash table
    custom_deleter(); 

    // reset jsonBuffer
    jsonBuffer.clear();

    // make outgoing message ready to add new data
    DATAroot = jsonBuffer.createObject();
    create_object("data", DATAroot);
}