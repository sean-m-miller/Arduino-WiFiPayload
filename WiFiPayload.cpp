#include "Arduino.h"
#include "WiFiPayload.h"

bool connected = false;

WiFiPayload::WiFiPayload(WiFiUDP& u) : udp(u){
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

    data->DATAroot["msg_type"] = "transmission";
    data->DATAroot["ip"] = WiFi.localIP().toString();
    data->DATAroot["device_name"] = device_name;
    
    //data->root.set<unsigned long>("timestamp", ts);

    mes_length = data->jsonBuffer.size(); // returns number of bytes being used in the array by jsonObjects. 
    // Set so that write_to_circ has access to it (data object might be destroyed before )

    data->DATAroot.printTo(mes_buf, 1024); // convert from JSON object to c string

    write_to_circ();
    clear_data();
}

void WiFiPayload::write_to_circ(){
    Serial.print("Before Writing");
    Serial.print(mes_buf);
    delay(4000);
    for(size_t i = 0; i < mes_length; i++){
        if((in + 4) % circ_length == out){ // + 4 to accommodate the flag
            return; // what should happen in the event that in catches up with out..?
        }
        circ_buf[in] = mes_buf[i];
        in = (in + 1) % circ_length;
    }
    circ_buf[(in + 1) % circ_length] = ':';
    circ_buf[(in + 2) % circ_length] = '^';
    circ_buf[(in + 3) % circ_length] = '_';
    circ_buf[(in + 4) % circ_length] = '^';
    in = (in + 5) % circ_length; // incriment in to be one index after end of flag.
}

int WiFiPayload::read_from_circ(){
    while(out != in){
        char temp[1024];
        size_t index = 0;
        while(!((circ_buf[out] == ':') && (circ_buf[(out + 1) % circ_length] == '^') && (circ_buf[(out + 2) % circ_length] == '_') && (circ_buf[(out + 3) % circ_length] == '^'))){
            temp[index] = circ_buf[out];
            out = (out + 1) % circ_length;
            index++;
        }
        out = (out + 4) % circ_length;
        Serial.print("After Reading");
        Serial.print(temp);
        delay(4000);
        udp.beginPacket(udpAddress,udpPort);
        udp.printf(temp);
        return udp.endPacket();
    }
}

void WiFiPayload::heartbeat(){ // handle all "asynchronous" tasks -> read and send data from circ_buf, send heartbeat every 3 seconds
    int sec = second();
    if((sec != time) && (sec % 3 == 0)){ // if its not the same second as last heartbeat, and if its 3 seconds after the last heartbeat.
        time = sec;
        char heart_buf[100];

        StaticJsonBuffer<100> tempBuffer; // buffer on stack. Use DynamicJsonBuffer for buffer on heap.
        JsonObject& temp_root = tempBuffer.createObject(); // initialize root of JSON object. Memory freed when root goes out of scope.
        temp_root.set<String>("msg_type", "heartbeat");
        temp_root.set<String>("ip", WiFi.localIP().toString());

        temp_root.printTo(heart_buf, 100);

        //Send a packet
        Serial.print(" HEARTBEAT: ");
        Serial.print(heart_buf);
        udp.beginPacket(udpAddress, udpPort);
        udp.printf(heart_buf);
        udp.endPacket();
    }
    read_from_circ();
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

