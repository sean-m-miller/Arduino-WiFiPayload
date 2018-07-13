#include "Arduino.h"
#include "WiFiPayload.h"

bool connected = false;

WiFiUDP udp;

WiFiPayload::WiFiPayload(){
    //Serial.println("in WiFiPayload constructor");
    data = new Data;/*new_data;*/
    data->create_custom_object("data", data->DATAroot);
}

WiFiPayload::~WiFiPayload(){
    delete data;
}

void WiFiPayload::set_device_name(String name){
    if(name.length() < 25){ //arbitrary max length for device name
        device_name = name;
    }
}

void WiFiPayload::write(){

    data->DATAroot["msg_type"] = "transmission"; // add to root object, not 
    data->DATAroot["ip"] = WiFi.localIP().toString();
    data->DATAroot["device_name"] = device_name;
    
    //data->root.set<unsigned long>("timestamp", ts);

    mes_length = data->jsonBuffer.size(); // returns number of bytes being used in the array by jsonObjects. 
    // Set so that write_to_circ has access to it (data object might be destroyed before )

    data->DATAroot.printTo(mes_buf, 1024); // convert from JSON object to c string

    write_to_buf();
    clear_data();
}

void WiFiPayload::write_to_buf(){
    buf.write(mes_buf, mes_length);
}

int WiFiPayload::read_from_buf(){
    buf.read(udp, udpAddress, udpPort);
}

void WiFiPayload::heartbeat(){ // handle all "asynchronous" tasks -> read and send data from circ_buf, send heartbeat every 3 seconds
    // int sec = second();
    // if((sec != time) && (sec % 3 == 0)){ // if its not the same second as last heartbeat, and if its 3 seconds after the last heartbeat.
        //time = sec;
        char heart_buf[68]; // largest possible heartbeat message: 192.168.1.143 used 66 bytes, if ip in form WWW.XXX.YYY.ZZZ, would use two more bytes. 

        StaticJsonBuffer<68> tempBuffer; // buffer on stack. Use DynamicJsonBuffer for buffer on heap.
        JsonObject& temp_root = tempBuffer.createObject(); // initialize root of JSON object. Memory freed when root goes out of scope.
        temp_root.set<String>("msg_type", "heartbeat");
        temp_root.set<String>("ip", WiFi.localIP().toString());

        temp_root.printTo(heart_buf, 68);

        //write to circ_buf
        buf.write(heart_buf, 68); // no null characters in this message ever besides the terminating one hopefully
        //delay(10000);
    // }
    read_from_buf();
}

const char* WiFiPayload::get_networkName(){
    return networkName;
}

const char* WiFiPayload::get_networkPswd(){
    return networkPswd;
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

    //register event handler
    WiFi.onEvent(WiFiEvent);

    //udp.begin(WiFi.localIP(),udpPort); only needed for listening, which we don't need yet.
  
    //Initiate connection
    WiFi.begin(networkName, networkPswd); // password must be chars with ASCII values between 32-126 (decimal)

    Serial.println("Waiting for WIFI connection...");
}

void WiFiPayload::clear_data(){
    delete data;
    data = new Data; // set to address of new_data
    data->create_custom_object("data", data->DATAroot);
}

void WiFiPayload::create_custom_object(const char* key){
    data->create_custom_object(key, data->find_custom_object("data"));
}

void WiFiPayload::create_array(const char* key){
    data->create_array(key, data->find_custom_object("data"));
}

size_t WiFiPayload::get_capacity(){
    return data->jsonBuffer.size();
}