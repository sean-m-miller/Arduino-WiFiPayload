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

    write_mes_buf[0] = (calculated_crc >> 3*8) & 255;
    write_mes_buf[1] = (calculated_crc >> 2*8) & 255;
    write_mes_buf[2] = (calculated_crc >> 8) & 255;
    write_mes_buf[3] = calculated_crc & 255;

    Serial.println("MESSAGE BEING WRITTEN TO DDS");

    Serial.println(write_mes_buf);

    size_t mes_length = (strlen(&write_mes_buf[4]) + 4);

    // for(int i = 0; i < mes_length; i++){
    //     Serial.println((uint8_t)write_mes_buf[i]);
    // }

    write_buf.receive_out(write_mes_buf, mes_length);
    out_data.clear();

    out_data.DATAroot["msg_type"] = "transmission"; // add to root object, not 
    out_data.DATAroot["ip"] = WiFi.localIP().toString();
    out_data.DATAroot["device_name"] = device_name;

    return 0;
}

int WiFiPayload::read_into_buf(){ 

    // if a packet has been recieved, write it into read_buf
    int packetSize = udp.parsePacket();
    int count = 0;
    if (packetSize){
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

int WiFiPayload::read(){ // clears current read message. return size of first message in read_buf, and load into read_mes_buf

    in_data.clear(); // with this setup, in_data gets cleared even if no hash table exists. Make clear() virtual, with the Incoming_Data version only clearing if ready = true, and flipping ready to false after. 
    
    Serial.println("VALUE OF extract_message()");
        
    size_t message_size = read_buf.send_message(read_mes_buf);

    if(message_size){ // a message was read
        
        // extract crc
        uint8_t incoming_crc[4];

        for(size_t i = 0; i < 4; i++){
            incoming_crc[i] = read_mes_buf[i];
        }

        uint32_t claimed_crc = incoming_crc[0] << 3*8 | incoming_crc[1] << 2*8 | incoming_crc[2] << 8 | incoming_crc[3];

        Serial.println("message from DDS:");

        Serial.println(strlen(&read_mes_buf[4]));

        FastCRC32 CRC32;

        uint32_t calculated_crc = CRC32.crc32((uint8_t*)&read_mes_buf[4], strlen(&read_mes_buf[4]));

        in_data.DATAroot = in_data.jsonBuffer.parseObject(&read_mes_buf[4], 50);

        if(claimed_crc != calculated_crc){
            Serial.println("CRC DID NOT MATCH");
            Serial.println(claimed_crc);
            Serial.println(calculated_crc);
            return -2;
        }

        //Serial.println("before create obj map line 101");

        if(in_data.DATAroot.success()){ // checks for valid JSON
            ready = true; // set ready true, so that if create_obj_map fails, clear() from the next read() will reset hash table
            in_data.create_obj_map(in_data.DATAroot); // only want user_generated fields
            if(in_data.DATAroot.success()){
               // Serial.println("line 156");
                ready = true;
                return 0; // obj_map and in_data have been successfully created out of incoming message.
            }
            ready = false;
            return -5; // unintended internal error
        }
    }
    return -1;
}

int WiFiPayload::parse_array_cstring(const char* key_, size_t index, char* destination){ // custom array overload
    if(ready){
        return in_data.parse_array_cstring(key_, index, destination);
    }
    return -4;
}

int WiFiPayload::parse_object_cstring(const char* key_, const char* index, char* destination){ // custom object overload
    if(ready){
        return in_data.parse_object_cstring(key_, index, destination);
    }
    return -4;
}

int WiFiPayload::parse_data_cstring(const char* key_, char* destination){ // data field overload
    if(ready){
        return in_data.parse_data_cstring(key_, destination);
    }
    return -4;
}

void WiFiPayload::heartbeat(){ // handle all "asynchronous" tasks -> read and send data from circ_buf, send heartbeat every 3 seconds

    if(myWiFi->status() == WL_CONNECTED){
        connected = true;

        char heart_buf[110]; // largest possible heartbeat message: 192.168.1.143 used 66 bytes, if ip in form WWW.XXX.YYY.ZZZ, would use two more bytes. 

        ///Serial.println("line 147 in heartbeat");

        StaticJsonBuffer<100> tempBuffer; // buffer on stack. Use DynamicJsonBuffer for buffer on heap.
        JsonObject& temp_root = tempBuffer.createObject(); // initialize root of JSON object. Memory freed when root goes out of scope.
        temp_root.set<String>("msg_type", "heartbeat");
        temp_root.set<String>("ip", WiFi.localIP().toString());
        temp_root.set<String>("device_name", device_name);

       // Serial.println("line 155 in heartbeat");

        temp_root.printTo(heart_buf);

        FastCRC32 CRC32;

        uint32_t calculated_crc = CRC32.crc32((uint8_t*) heart_buf, strlen(heart_buf));

        heart_buf[0] = (calculated_crc >> 3*8) & 255;
        heart_buf[1] = (calculated_crc >> 2*8) & 255;
        heart_buf[2] = (calculated_crc >> 8) & 255;
        heart_buf[3] = calculated_crc & 255;

        //Serial.println("line 161 in heartbeat");

        temp_root.printTo(&heart_buf[4], tempBuffer.size()); // convert from JSON object to c string

        // for(int i = 0; i < tempBuffer.size() + 4; i++){
        //     Serial.println((uint8_t)heart_buf[i]);
        // }

        Serial.println("HEARTBEAT BEING SENT TO DDS");

        Serial.println(heart_buf);

        //write to circ_buf
        write_buf.receive_out(heart_buf, tempBuffer.size() + 4); // no null characters in this message ever besides the terminating one hopefully
        
        write_buf.send_out(udp, udpAddress, udpPort);

        read_into_buf();
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
