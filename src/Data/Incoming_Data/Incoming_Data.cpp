#include "Arduino.h" 
#include "Incoming_Data.hpp"

Incoming_Data::Incoming_Data(){
    for(size_t i = 0; i < 100; i++){
        obj_map[i] = NULL;
    }
}

void Incoming_Data::create_obj_map(JsonVariant obj){
    for(JsonPair& pair : obj.as<JsonObject>()){
        if(pair.value.is<JsonObject&>()){
            create_object(pair.key, pair.value.as<JsonObject>());
            create_obj_map(pair.value);
        }
        if(pair.value.is<JsonArray&>()){
            create_array(pair.key, pair.value.as<JsonArray>());

            // update new array's capacity
            Node* it = find_custom(pair.key);
            it->capacity = pair.value.as<JsonArray>().size();
        }
    }
}

int Incoming_Data::create_object(const char* key_, JsonObject& custom){
    int index = hash(key_);

    // element already mapped there
    if(Node* it = obj_map[index]){ 
        while(it->next!=NULL){
            if(!strcmp(it->key, key_)){
                return -1; // key error that identifier already exists
            }
            it = it->next;
        }

        // check tail
        if(!strcmp(it->key, key_)){ 
            return -1;
        }

        // creates Node out of custom, without calling createNestedObject() in constructor.
        it->next = new Custom_Object(key_, custom); 
    }
    else{

        // creates Node out of custom, without calling createNestedObject() in constructor.
        obj_map[index] = new Custom_Object(key_, custom); 
    }
    return 0; // success
}

int Incoming_Data::create_array(const char* key_, JsonArray& custom){
    int index = hash(key_);

    // element already mapped there
    if(Node* it = obj_map[index]){ 
        while(it->next!=NULL){
            if(!strcmp(it->key, key_)){
                return -1; //key error that identifier already exists
            }
            it = it->next;
        }

        //check tail
        if(!strcmp(it->key, key_)){ 
            return -1;
        }

        // creates Node out of custom, without calling createNestedArray() in constructor.
        it->next = new Custom_Array(key_, custom);
    }
    else{

        // creates Node out of custom, without calling createNestedArray() in constructor.
        obj_map[index] = new Custom_Array(key_, custom);
    }
    return 0;
}

int Incoming_Data::parse_array_cstring(const char* key_, size_t index, char* destination){
    if(Node* it = find_custom(key_)){
        if(it->var.is<JsonArray>()){
            if(index < it->capacity){
                strcpy(destination, it->get_field()[index]);
                return 0; // success
            }
            return -1; // index does not exist
        }
        return -3; // key_ names an object
    }
    return -2; // custom "key_" does not exist
}

int Incoming_Data::parse_object_cstring(const char* key_, const char* index, char* destination){
    if(Node* it = find_custom(key_)){
        if(it->var.is<JsonObject>()){
            if(it->get_field().as<JsonObject&>().containsKey(key_)){
                strcpy(destination, it->get_field()[index]);
                return 0; // success
            }
            return -1; // index does not exist
        }
        return -3; // key_ names an array
    }
    return -2; // custom "key_" does not exist 
}

int Incoming_Data::parse_data_cstring(const char* index, char* destination){
    if(Node* it = find_custom("data")){
        if(it->var.is<JsonObject>()){
            if(it->get_field().as<JsonObject&>().containsKey(index)){
                strcpy(destination, it->get_field()[index]);
                return 0; // success
            }
            return -1; // key not found
        }
        return -5; // unintended internal error
    }
    return -5; // unintended internal error
}

void Incoming_Data::clear(){

    // reset hash table
    custom_deleter(); 

    // reset jsonBuffer
    jsonBuffer.clear(); 
}