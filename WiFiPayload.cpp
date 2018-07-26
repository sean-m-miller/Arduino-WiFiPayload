#include "Arduino.h"
#include "WiFiPayload.h"

bool connected = false;

//WiFiUDP udp;

WiFiPayload::WiFiPayload(){}

WiFiPayload::~WiFiPayload(){}

void WiFiPayload::set_device_name(const char* name){
    if(strlen(name) < 25){ //arbitrary max length for device name
        strcpy(device_name, name);
    }
}

void WiFiPayload::print_out_data_to(char* destination, size_t size){
    out_data.print_to(destination, size);
}

void WiFiPayload::print_in_data_to(char* destination, size_t size){
    in_data.print_to(destination, size);
}

size_t WiFiPayload::add_array(const char* key_){
    return out_data.add_array(key_);
}

size_t WiFiPayload::add_object(const char* key_){
    return out_data.add_object(key_);
}

size_t WiFiPayload::create_nested_array(const char* key_, const char* arr_key){
    return out_data.create_nested_array(key_, arr_key);
}

size_t WiFiPayload::create_nested_object(const char* key_, const char* obj_key){
    return out_data.create_nested_object(key_, obj_key);
}

size_t WiFiPayload::write(){

    if(!strcmp(device_name, "no_name")){
        return 0;
    }

    out_data.DATAroot["msg_type"] = "transmission"; // add to root object, not 
    out_data.DATAroot["ip"] = WiFi.localIP().toString();
    out_data.DATAroot["device_name"] = device_name;
    
    //data->root.set<unsigned long>("timestamp", ts);

    mes_length = out_data.jsonBuffer.size(); // returns number of bytes being used in the array by jsonObjects. 
    // Set so that write_to_circ has access to it (data object might be destroyed before )

    out_data.DATAroot.printTo(write_mes_buf, mes_length); // convert from JSON object to c string

    write_buf.write(write_mes_buf, mes_length);
    out_data.clear();
    out_data.DATAroot = out_data.jsonBuffer.createObject();
    if(!out_data.DATAroot.success()){
        Serial.println("Outgoing_constructor failed, JsonObject not created out of jsonBuffer");
        return 0;
    }
    out_data.create_object("data", out_data.DATAroot); // have this call here since clearing an Incoming message should not add a data object after resetting hash table.
    return 1;
}

int WiFiPayload::read_into_buf(){ // if a packet has been recieved, write it into read_buf
    int packetSize = udp.parsePacket();
    size_t count = 0;
    if (packetSize){
        Serial.println("packetsize isssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss:");
        Serial.println(packetSize);
        if(read_buf.checkCapacity(packetSize)){
            read_buf.start_msg();        
            for(size_t i = 0; i < packetSize; i++){

                read_buf.write_char(udp.read());

                count ++;

            }
            read_buf.end_msg();
        }
    }
    return count;   
}

size_t WiFiPayload::read(){ // clears current read message. return size of first message in read_buf, and load into read_mes_buf

    //memset(read_mes_buf, '/0', MESSAGE_SIZE);

    in_data.clear(); // with this setup, in_data gets cleared even if no hash table exists. Make clear() virtual, with the Incoming_Data version only clearing if ready = true, and flipping ready to false after. 
    
    Serial.println("VALUE OF extract_message()");
        
    size_t y = read_buf.extract_message(read_mes_buf);

    Serial.println(y);

    in_data.DATAroot = in_data.jsonBuffer.parseObject(read_mes_buf);

    Serial.println("before create obj map line 101");

    int x = in_data.DATAroot["data"]["b"];
    Serial.println(x);

    if(in_data.DATAroot.success()){ // checks for valid JSON
        x = in_data.DATAroot["data"]["b"];
        Serial.println(x);
        in_data.create_obj_map(in_data.DATAroot);
        x = in_data.DATAroot["data"]["b"];
        Serial.println(x);
        if(in_data.DATAroot.success()){
            return 1;
        }
    }
    return 0;
}

size_t WiFiPayload::parse_array_string(const char* key_, size_t index, char* destination){ // custom array overload
    return in_data.parse_array_string(key_, index, destination);
}

size_t WiFiPayload::parse_object_string(const char* key_, const char* index, char* destination){ // custom object overload
    return in_data.parse_object_string(key_, index, destination);
}

size_t WiFiPayload::parse_data_string(const char* key_, char* destination){ // data field overload
    return in_data.parse_data_string(key_, destination);
}

void WiFiPayload::heartbeat(){ // handle all "asynchronous" tasks -> read and send data from circ_buf, send heartbeat every 3 seconds
    // int sec = second();
    // if((sec != time) && (sec % 3 == 0)){ // if its not the same second as last heartbeat, and if its 3 seconds after the last heartbeat.
        //time = sec;
    char heart_buf[100]; // largest possible heartbeat message: 192.168.1.143 used 66 bytes, if ip in form WWW.XXX.YYY.ZZZ, would use two more bytes. 

    StaticJsonBuffer<100> tempBuffer; // buffer on stack. Use DynamicJsonBuffer for buffer on heap.
    JsonObject& temp_root = tempBuffer.createObject(); // initialize root of JSON object. Memory freed when root goes out of scope.
    temp_root.set<String>("msg_type", "heartbeat");
    temp_root.set<String>("ip", WiFi.localIP().toString());
    temp_root.set<String>("device_name", device_name);

    temp_root.printTo(heart_buf, 100);

    //write to circ_buf
    write_buf.write(heart_buf, 100); // no null characters in this message ever besides the terminating one hopefully
        //delay(10000);
    // }
    write_buf.read(udp, udpAddress, udpPort);

    read_into_buf();
}

//wifi event handler
void WiFiPayload::WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP()); 
          
          //initializes the UDP state
          //This initializes the transfer buffer
          
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;

    //   case SYSTEM_EVENT_STA_LOST_IP:
    //       Serial.println("Connected, but IP Reset, so ")
    }
}

void WiFiPayload::connectToWiFi(){
    Serial.println("Connecting to WiFi network: " + String(networkName));
    
    // delete old config
    Serial.print(WiFi.disconnect(true));

    Serial.println("udp.begin()");
    Serial.println(udp.begin(udpPort));

    //register event handler
    WiFi.onEvent(WiFiEvent);
  
    //Initiate connection
    WiFi.begin(networkName, networkPswd); // password must be chars with ASCII values between 32-126 (decimal)

    Serial.println("Waiting for WIFI connection...");
}

// size_t WiFiPayload::write_in_msg(){

//     in_data.add_data("msg_type", "transmission"); // add to root object, not 
//     in_data.add_data("ip", WiFi.localIP().toString());
//     in_data.add_data("device_name", device_name);
    
//     //data->root.set<unsigned long>("timestamp", ts);

//     mes_length = in_data.jsonBuffer.size(); // returns number of bytes being used in the array by jsonObjects. 
//     // Set so that write_to_circ has access to it (data object might be destroyed before )

//     in_data.DATAroot.printTo(write_mes_buf, mes_length); // convert from JSON object to c string

//     write_buf.write(write_mes_buf, mes_length);
//     in_data.clear();
    
//     return 1;
// }