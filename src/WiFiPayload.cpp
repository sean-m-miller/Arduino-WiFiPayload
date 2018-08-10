#include "Arduino.h"
#include <WiFiPayload.h>

WiFiPayload::WiFiPayload(const char* ssid, const char* pswd, const char* udpAdd, const int udpP) : udpPort(udpP) {
    strcpy(networkName, ssid);
    strcpy(networkPswd, pswd);
    strcpy(udpAddress, udpAdd);
}

WiFiPayload::~WiFiPayload(){}

int WiFiPayload::set_device_name(const char* name){
    if(strlen(name) < values::NAME_SIZE){
        strcpy(device_name, name);
        return 0;
    }
    return -1;
}

int WiFiPayload::begin(){

    // if device name has been set
    if(strcmp("No_Name", device_name)){ 

        // class pointer to extern WiFiClass object
        myWiFi = &WiFi;
        connectToWiFi();

        // make out_data ready to accept user input ("data" field gets added to DATAroot in the out_data constructor)
        out_data.DATAroot["msg_type"] = "transmission";
        out_data.DATAroot["ip"] = WiFi.localIP().toString();
        out_data.DATAroot["device_name"] = device_name;
        return 0; // success
    }
    return -1; // device_name not set
}

size_t WiFiPayload::out_msg_size(){
    return out_data.msg_size();
}

size_t WiFiPayload::out_msg_space_left(){

    // amount of space left in the outgoing message. + 4 for crc
    return(values::MESSAGE_SIZE - (out_data.msg_size() + 4)); // + 4 for crc
}

size_t WiFiPayload::in_msg_size(){
    return in_data.msg_size();
}

bool WiFiPayload::is_connected(){

    // connected getter function
    return connected;
}

void WiFiPayload::print_out_data_to(char* destination){

    size_t size_of_data = out_msg_size();

    // prints outgoing message to destination buffer
    out_data.DATAroot.printTo(destination, size_of_data);
}

void WiFiPayload::print_in_data_to(char* destination){

    size_t size_of_data = in_msg_size();

    // prints incoming message to destination
    in_data.DATAroot.printTo(destination, size_of_data);
}

int WiFiPayload::add_array(const char* key_){
    return out_data.add_array(key_);
}

int WiFiPayload::add_object(const char* key_){
    return out_data.add_object(key_);
}

int WiFiPayload::create_nested_array(const char* key_, const char* arr_key){
    return out_data.create_nested_array(key_, arr_key);
}

int WiFiPayload::create_nested_object(const char* key_, const char* obj_key){
    return out_data.create_nested_object(key_, obj_key);
}

int WiFiPayload::write(){

    if(!strcmp(device_name, "No_Name")){
        out_data.clear();

        out_data.DATAroot["msg_type"] = "transmission";
        out_data.DATAroot["ip"] = WiFi.localIP().toString();
        out_data.DATAroot["device_name"] = device_name;

        return -1;
    }

    // generate crc hash https://arduinojson.org/v5/doc/tricks/

    out_data.DATAroot.printTo(&write_mes_buf[4], out_data.jsonBuffer.size()); // includes null char

    FastCRC32 CRC32;

    uint32_t calculated_crc = CRC32.crc32((uint8_t*) &write_mes_buf[4], strlen(&write_mes_buf[4])); // on my machine, char* and uint8_t* are NOT compatible

    // insert into first 4 bytes of write_mes_buf
    write_mes_buf[0] = (calculated_crc >> 3*8) & 255;
    write_mes_buf[1] = (calculated_crc >> 2*8) & 255;
    write_mes_buf[2] = (calculated_crc >> 8) & 255;
    write_mes_buf[3] = calculated_crc & 255;

    Serial.println("MESSAGE BEING WRITTEN TO DDS");

    Serial.println(write_mes_buf);

    // do not use strlen on index zero of write_mes_buf in the event that one of the 4 crc bytes evaluates to null char
    size_t mes_length = (strlen(&write_mes_buf[4]) + 4);

    write_buf.receive_out(write_mes_buf, mes_length);
    out_data.clear();

    out_data.DATAroot["msg_type"] = "transmission";
    out_data.DATAroot["ip"] = WiFi.localIP().toString();
    out_data.DATAroot["device_name"] = device_name;

    return 0;
}

int WiFiPayload::receive_into_read_buf(){ 

    // if a packet has been recieved, write it into read_buf
    int packetSize = udp.parsePacket();
    int count = 0;
    if (packetSize){

        // make sure message received is less than maximum message size
        if(read_buf.checkSize(packetSize)){
            read_buf.start_msg();        
            for(size_t i = 0; i < packetSize; i++){
                read_buf.receive_char(udp.read());
                count ++;
            }
            read_buf.end_msg();
        }
    }
    return count;
}

int WiFiPayload::read(){

    in_data.clear();

    ready = false; 
    
    Serial.println("VALUE OF extract_message()");
    
    // pop message of read_buf and copy to read_mes_buf
    size_t message_size = read_buf.send_message(read_mes_buf);

    // if we copied a message
    if(message_size){
        
        // extract crc
        uint8_t incoming_crc[4];
        for(size_t i = 0; i < 4; i++){
            incoming_crc[i] = read_mes_buf[i];
        }
        uint32_t claimed_crc = incoming_crc[0] << 3*8 | incoming_crc[1] << 2*8 | incoming_crc[2] << 8 | incoming_crc[3];

        Serial.println("message from DDS:");
        Serial.println(strlen(&read_mes_buf[4]));

        // calculate crc
        FastCRC32 CRC32;
        uint32_t calculated_crc = CRC32.crc32((uint8_t*)&read_mes_buf[4], strlen(&read_mes_buf[4]));
        in_data.DATAroot = in_data.jsonBuffer.parseObject(&read_mes_buf[4], 50);

        if(claimed_crc != calculated_crc){
            Serial.println("CRC DID NOT MATCH");
            Serial.println(claimed_crc);
            Serial.println(calculated_crc);
            return -2;
        }

        //check for valid json, construct obj_map, and set ready accordingly
        if(in_data.DATAroot.success()){
            ready = true;
            in_data.create_obj_map(in_data.DATAroot["data"]); // only want user generated fields in obj_map, not "ip", "msg_type", etc.
            if(in_data.DATAroot.success()){
                ready = true;
                return 0; // success
            }
            ready = false;
            return -5; // unintended internal error
        }
    }
    return -1;
}

int WiFiPayload::parse_array_cstring(const char* key_, size_t index, char* destination){
    if(ready){
        return in_data.parse_array_cstring(key_, index, destination);
    }
    return -4;
}

int WiFiPayload::parse_object_cstring(const char* key_, const char* index, char* destination){
    if(ready){
        return in_data.parse_object_cstring(key_, index, destination);
    }
    return -4;
}

int WiFiPayload::parse_data_cstring(const char* key_, char* destination){
    if(ready){
        return in_data.parse_data_cstring(key_, destination);
    }
    return -4;
}

void WiFiPayload::heartbeat(){

    // check to see if still connected
    if(myWiFi->status() == WL_CONNECTED){
        connected = true;

        //construct heartbeat message
        char heart_buf[110]; 
        StaticJsonBuffer<100> tempBuffer;
        JsonObject& temp_root = tempBuffer.createObject();
        temp_root.set<String>("msg_type", "heartbeat");
        temp_root.set<String>("ip", WiFi.localIP().toString());
        temp_root.set<String>("device_name", device_name);
        temp_root.printTo(heart_buf);

        // calculate crc and add to first 4 bytes of heart_buf
        FastCRC32 CRC32;
        uint32_t calculated_crc = CRC32.crc32((uint8_t*) heart_buf, strlen(heart_buf));
        heart_buf[0] = (calculated_crc >> 3*8) & 255;
        heart_buf[1] = (calculated_crc >> 2*8) & 255;
        heart_buf[2] = (calculated_crc >> 8) & 255;
        heart_buf[3] = calculated_crc & 255;

        // copy object back into heartbeat, overwriting the previous copy (offset by 4 from crc)
        temp_root.printTo(&heart_buf[4], tempBuffer.size()); 

        Serial.println("HEARTBEAT BEING SENT TO DDS");
        Serial.println(heart_buf);

        // write to write_buf
        write_buf.receive_out(heart_buf, tempBuffer.size() + 4); // no null characters in this message ever besides the terminating one hopefully
        
        // send all messages that have been added to write_buf
        write_buf.send_out(udp, udpAddress, udpPort);

        // if messages sent from DDS, write them into read_buf
        receive_into_read_buf();
    }
    else{
        connected = false;

        // retry connection..? Hard to test if this is necessary. Check to see if WiFi.begin() tells WiFi to retry connection if it disconnects.
        //connectToWiFi(); 
    }
}

void WiFiPayload::connectToWiFi(){
    Serial.println("Connecting to WiFi network: " + String(networkName));
    
    // delete old config
    Serial.println(myWiFi->disconnect(true));

    //Serial.println("udp.begin()");
    Serial.println(udp.begin(udpPort));
  
    //Initiate connection
    myWiFi->begin(networkName, networkPswd); // password must be chars with ASCII values between 32-126 (decimal)

    Serial.println("Waiting for WIFI connection...");
}
