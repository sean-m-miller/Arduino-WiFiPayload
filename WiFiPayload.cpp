#include "Arduino.h"
#include "WiFiPayload.h"

bool connected = false;

WiFiPayload::WiFiPayload(WiFiUDP& u) : udp(u) {

    data = new Data;

    //connectToWiFi();

    // //attempt to initialize nvs flash memory. From: https://github.com/espressif/esp-idf/blob/master/examples/wifi/simple_wifi/main/simple_wifi.c
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
    //   ESP_ERROR_CHECK(nvs_flash_erase());
    //   ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(ret);
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
    // data.root.set<String>("msg_type", "transmission"); //explicit casts for clarity
    // data.root.set<String>("ip", WiFi.localIP().toString());
    // data.root.set<String>("device_name", device_name);
     

    data->add("msg_type", "transmission");
    data->add("ip", WiFi.localIP().toString());
    data->add("device_name", device_name);

    //data->root.set<unsigned long>("timestamp", ts);

    mes_length = data->jsonBuffer.size(); // returns number of bytes being used in the array by jsonObjects. 
    // Set so that write_to_circ has access to it (data object might be destroyed before )

    data->root.printTo(mes_buf, 1024); // convert from JSON object to c string

    write_to_circ();

    //Send a packet -> MOVED TO READ_FROM_CIRC()
    // udp.beginPacket(udpAddress,udpPort);
    // udp.printf(mes_buf);
    // return udp.endPacket(); // 1 or 0
    // message too big
    //Serial.print("Write Failed: not connected");
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

    //while(!connected){}

    //udp.begin(WiFi.localIP(),udpPort); // 
}

void WiFiPayload::clear_data(){
    delete data;
    data = new Data;
}