#include "Arduino.h"
#include "WiFiPayload.h"

bool connected = false; // wrap with function WiFiPayload::connected(), keep connected private in WiFiPayload

WiFiPayload::WiFiPayload(const char* ssid, const char* pswd, const char* udpAdd, const int udpP) : udpPort(udpP) {
    strcpy(networkName, ssid);
    strcpy(networkPswd, pswd);
    strcpy(udpAddress, udpAdd);
}

WiFiPayload::~WiFiPayload(){}

void WiFiPayload::begin(){
    myWiFi = &WiFi; 
    connectToWiFi();
}

size_t WiFiPayload::out_msg_size(){
    return out_data.msg_size() + values::DEAD_SPACE;
}

size_t WiFiPayload::in_msg_size(){
    return in_data.msg_size();
}

bool WiFiPayload::is_connected(){
    return connected;
}

int WiFiPayload::set_device_name(const char* name){
    if(strlen(name) < 25){ //arbitrary max length for device name
        strcpy(device_name, name);
    }
    return -1;
}

void WiFiPayload::print_out_data_to(char* destination, size_t size){
    out_data.print_to(destination, size);
}

void WiFiPayload::print_in_data_to(char* destination, size_t size){
    in_data.print_to(destination, size);
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

    if(!strcmp(device_name, "no_name")){
        return -1;
    }

    out_data.DATAroot["msg_type"] = "transmission"; // add to root object, not 
    out_data.DATAroot["ip"] = WiFi.localIP().toString();
    out_data.DATAroot["device_name"] = device_name;

    // generate crc hash https://arduinojson.org/v5/doc/tricks/

    out_data.DATAroot.printTo(&write_mes_buf[4], strlen(&write_mes_buf[4]));

    FastCRC32 CRC32;

    uint32_t calculated_crc = CRC32.crc32((uint8_t*) &write_mes_buf[4], strlen(&write_mes_buf[4])); // on my machine, char* and uint8_t* are NOT compatible

    write_mes_buf[0] = (calculated_crc >> 3*8) & 255;
    write_mes_buf[1] = (calculated_crc >> 2*8) & 255;
    write_mes_buf[2] = (calculated_crc >> 8) & 255;
    write_mes_buf[3] = calculated_crc & 255;

    Serial.println("MESSAGE BEING WRITTEN TO DDS");

    Serial.println(write_mes_buf);

    mes_length = (strlen(write_mes_buf)); // returns number of bytes being used in the array by jsonObjects. 

    write_buf.receive_out(write_mes_buf, mes_length);
    out_data.clear();
    return 0;
}

int WiFiPayload::read_into_buf(){ // if a packet has been recieved, write it into read_buf
    int packetSize = udp.parsePacket();
    size_t count = 0;
    if (packetSize){
        if(read_buf.checkCapacity(packetSize)){
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

        uint32_t claimed_crc = read_mes_buf[0] << 3*8 | read_mes_buf[1] << 2*8 | read_mes_buf[2] << 8 | read_mes_buf[3];

        Serial.println("message from DDS:");

        Serial.println(strlen(&read_mes_buf[4]));

        FastCRC32 CRC32;

        uint32_t calculated_crc = CRC32.crc32((uint8_t*) &read_mes_buf[4], strlen(&read_mes_buf[4]));

        in_data.DATAroot = in_data.jsonBuffer.parseObject(&read_mes_buf[4], 50);

        if(claimed_crc != calculated_crc){
            Serial.println("CRC DID NOT MATCH");
            Serial.println(claimed_crc);
            Serial.println(calculated_crc);
            return -1;
        }

        Serial.println("before create obj map line 101");

        if(in_data.DATAroot.success()){ // checks for valid JSON
            ready = true; // set ready true, so that if create_obj_map fails, clear() from the next read() will reset hash table
            in_data.create_obj_map(in_data.DATAroot);
            if(in_data.DATAroot.success()){
                return 0; // obj_map and in_data have been successfully created out of incoming message.
            }
        }
    }
    return -1;
}

int WiFiPayload::parse_array_string(const char* key_, size_t index, char* destination){ // custom array overload
    return in_data.parse_array_string(key_, index, destination);
}

int WiFiPayload::parse_object_string(const char* key_, const char* index, char* destination){ // custom object overload
    return in_data.parse_object_string(key_, index, destination);
}

int WiFiPayload::parse_data_string(const char* key_, char* destination){ // data field overload
    return in_data.parse_data_string(key_, destination);
}

void WiFiPayload::heartbeat(){ // handle all "asynchronous" tasks -> read and send data from circ_buf, send heartbeat every 3 seconds
    // int sec = second();
    // if((sec != time) && (sec % 3 == 0)){ // if its not the same second as last heartbeat, and if its 3 seconds after the last heartbeat.
        //time = sec;
    if(myWiFi->status() == WL_CONNECTED){
        connected = true;

        char heart_buf[110]; // largest possible heartbeat message: 192.168.1.143 used 66 bytes, if ip in form WWW.XXX.YYY.ZZZ, would use two more bytes. 

        Serial.println("line 147 in heartbeat");

        StaticJsonBuffer<100> tempBuffer; // buffer on stack. Use DynamicJsonBuffer for buffer on heap.
        JsonObject& temp_root = tempBuffer.createObject(); // initialize root of JSON object. Memory freed when root goes out of scope.
        temp_root.set<String>("msg_type", "heartbeat");
        temp_root.set<String>("ip", WiFi.localIP().toString());
        temp_root.set<String>("device_name", device_name);

        Serial.println("line 155 in heartbeat");

        temp_root.printTo(heart_buf);

        FastCRC32 CRC32;

        uint32_t calculated_crc = CRC32.crc32((uint8_t*) heart_buf, strlen(heart_buf));

        heart_buf[0] = (calculated_crc >> 3*8) & 255;
        heart_buf[1] = (calculated_crc >> 2*8) & 255;
        heart_buf[2] = (calculated_crc >> 8) & 255;
        heart_buf[3] = calculated_crc & 255;

        Serial.println("line 161 in heartbeat");

        temp_root.printTo(&heart_buf[4], tempBuffer.size()); // convert from JSON object to c string

        Serial.println("HEARTBEAT BEING SENT TO DDS");

        Serial.println(heart_buf);

        //write to circ_buf
        write_buf.receive_out(heart_buf, tempBuffer.size() + 4); // no null characters in this message ever besides the terminating one hopefully
        
        write_buf.send_out(udp, udpAddress, udpPort);

        read_into_buf();
    }
    else{
        connected = false;
    }

    
}

//wifi event handler
// void WiFiPayload::WiFiEvent(WiFiPayload* self, WiFiEvent_t event){
//     switch(event) {
//       case SYSTEM_EVENT_STA_GOT_IP:
//           //When connected set 
//           Serial.print("WiFi connected! IP address: ");
//           Serial.println(self->myWiFi->localIP()); 
          
//           //initializes the UDP state
//           //This initializes the transfer buffer
          
//           self->connected = true;
//           break;
//       case SYSTEM_EVENT_STA_DISCONNECTED:
//           Serial.println("WiFi lost connection");
//           self->connected = false;
//           break;

//     //   case SYSTEM_EVENT_STA_LOST_IP:
//     //       Serial.println("Connected, but IP Reset, so ")
//     }
// }

void WiFiPayload::connectToWiFi(){
    Serial.println("Connecting to WiFi network: " + String(networkName));
    
    // delete old config
    Serial.print(WiFi.disconnect(true));

    Serial.println("udp.begin()");
    Serial.println(udp.begin(udpPort));

    //register event handler

    //WiFi.onEvent(WiFiEvent(this));
  
    //Initiate connection
    WiFi.begin(networkName, networkPswd); // password must be chars with ASCII values between 32-126 (decimal)

    Serial.println("Waiting for WIFI connection...");
}

WiFiPayload::HashPrint::HashPrint(){
    _hash = _hasher.crc32(NULL, 0);
}

size_t WiFiPayload::HashPrint::write(uint8_t c){
    _hash = _hasher.crc32_upd(&c, 1);
}

uint32_t WiFiPayload::HashPrint::hash() const {
    return _hash;
}