#include "Arduino.h"
#include "Data.hpp"

size_t Data::msg_size(){
    return jsonBuffer.size();
}

Data::Node::Node(const char* key_, JsonVariant var_) : var(var_){
    strcpy(key, key_);
}

JsonVariant& Data::Node::get_field(){
    return var;
}

Data::Custom_Object::Custom_Object(const char* key_, JsonVariant& root_) : obj(root_.as<JsonObject&>().createNestedObject(key_)) {
    strcpy(key, key_);
    var = obj;
}

Data::Custom_Object::Custom_Object(const char* key_, JsonObject& root_) : obj(root_){
    strcpy(key, key_);
    var = obj;
}

Data::Custom_Array::Custom_Array(const char* key_, JsonVariant& root_) : arr(root_.as<JsonObject&>().createNestedArray(key_)) {
    strcpy(key, key_);
    var = arr;
}

Data::Custom_Array::Custom_Array(const char* key_, JsonArray& root_) : arr(root_){
    strcpy(key, key_);
    var = arr;
}

Data::Node* Data::find_custom(const char* key_){
    if(Node* it = obj_map[hash(key_)]){
        while(it != NULL){
            if(strcmp(it->key, key_) == 0){
                return it;
            }
            it = it->next;
        }
    }
    return NULL; // key not found
}


int Data::hash(const char* key_){ 

    // bad polynomial hash function, but it works for now

    int sum = 0;
    bool flag = true;
    for(size_t i = 0; i < strlen(key_); i++){
        if(flag){
            sum = sum + (int)(pow(key_[i], 2)) % 100; //casts double to int
        }
        else{
            sum = sum + (int)(pow(key_[i], 1)) % 100; //casts double to int
        }
        flag = !flag;
    }
    return sum % 100;
}

void Data::custom_deleter(){
    for(size_t i = 0; i < 100; i++){
        deleter_helper(&obj_map[i]);
    }
}

void Data::deleter_helper(Node** it){
    if(*it == NULL){
        return;
    }
    deleter_helper(&(*it)->next);
    delete *it;
    *it = NULL;
}

// use for testing and debugging
void Data::print_hash_table(){
    for(size_t i = 0; i < 100; i++){
        if(obj_map[i]){
            Serial.print("at index: ");
            Serial.println(i);
            Node* it = obj_map[i];
            while(it){
                Serial.println(it->key);
                it = it->next;
            }
        }
    }
}