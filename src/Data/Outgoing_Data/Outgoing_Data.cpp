#include "Arduino.h"
#include "Outgoing_Data.hpp"

Outgoing_Data::Outgoing_Data(){

    for(size_t i = 0; i < 100; i++){
        obj_map[i] = NULL; // ensure all map pointers are set to NULL
    }
    DATAroot = jsonBuffer.createObject();

    if(!DATAroot.success()){
        Serial.println("Outgoing_constructor failed, JsonObject not created out of jsonBuffer");
    }

    create_object("data", DATAroot);
}

Outgoing_Data::~Outgoing_Data(){
    custom_deleter();
}

int Outgoing_Data::create_object(const char* key_, JsonVariant& custom){
    if(!custom.is<JsonObject>() || !custom.success()){
        // createNestedObject and array test for -3 error at the higher level. This case should only be hit by add_obect() or add_array() if "data" is invalid
        return -5;
    }
    int index = hash(key_);
    if(Node* it = obj_map[index]){ // element already mapped there
        while(it->next!=NULL){
            if(!strcmp(it->key, key_)){
                return -2; //key error that identifier already exists
            }
            it = it->next;
        }
        if(!strcmp(it->key, key_)){ //check tail
            return -2;
        }
        it->next = new Custom_Object(key_, custom); // set to end of list at that index
    }
    else{
        obj_map[index] = new Custom_Object(key_, custom); // set to end of list at that index
    }

    // allocation failed from not enough space in buffer. Remove from obj_map
    Node* it2 = obj_map[index];
    if(!it2->var.success()){
        it2 = obj_map[index];
        while(strcmp(it2->next->key, key_)){
            it2 = it2->next;
        }
        delete it2->next;
        it2->next = NULL;
        return -5;
    }
    return 0;
}

int Outgoing_Data::create_array(const char* key_, JsonVariant& custom){
    if(!custom.is<JsonObject>() || !custom.success()){
        // createNestedObject and array test for -3 error at the higher level. This case should only be hit by add_obect() or add_array() if "data" is invalid
        return -5; 
    }
    int index = hash(key_);
    if(Node* it = obj_map[index]){ // element already mapped there
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
        it->next = new Custom_Array(key_, custom); // set to end of list at that index
    }
    else{
        obj_map[index] = new Custom_Array(key_, custom); // set to end of list at that index
    }

    // allocation failed from not enough space in buffer. Remove from obj_map
    Node* it2 = obj_map[index];
    if(!it2->var.success()){
        it2 = obj_map[index];
        while(strcmp(it2->next->key, key_)){
            it2 = it2->next;
        }
        delete it2->next;
        it2->next = NULL;
        return -5;
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
    custom_deleter(); // reset hash table
    jsonBuffer.clear(); // reset jsonBuffer
    DATAroot = jsonBuffer.createObject();
    create_object("data", DATAroot);
}