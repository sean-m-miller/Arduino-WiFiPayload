#include "Arduino.h"
#include "WiFiPayload.h"

bool connected = false; // wrap with function WiFiPayload::connected(), keep connected private in WiFiPayload

//WiFiUDP udp;

WiFiPayload::WiFiPayload(){}

WiFiPayload::~WiFiPayload(){}

void WiFiPayload::begin(){
    myWiFi = &WiFi; 
}

size_t WiFiPayload::msg_size(){
    return out_data.msg_size();
}

// bool WiFiPayload::is_connected(){
//     return connected;
// }

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
    out_data.DATAroot["ip"] = myWiFi->localIP().toString();
    out_data.DATAroot["device_name"] = device_name;

    // generate crc hash https://arduinojson.org/v5/doc/tricks/

    HashPrint crc;
    out_data.DATAroot.printTo(crc);
    uint32_t crc_hash = crc.hash();

    uint32_t copy = crc_hash;

    size_t crc_digits = 0;

    while(copy != 0){
        crc_digits++;
        copy = copy / 10;
    }

    for(size_t i = 0; i < crc_digits; i++){
        write_mes_buf[(crc_digits-1)-i] = (crc_hash % 10) + '0'; // plus '0' to convert to ascii digits
        crc_hash = crc_hash / 10;
    }

    out_data.DATAroot.printTo(&write_mes_buf[crc_digits], out_data.jsonBuffer.size()); // convert from JSON object to c string

    Serial.println("MESSAGE BEING WRITTEN TO DDS");

    Serial.println(write_mes_buf);

    mes_length = (out_data.jsonBuffer.size() + crc_digits); // returns number of bytes being used in the array by jsonObjects. 

    write_buf.receive_out(write_mes_buf, mes_length);
    out_data.clear();
     // have this call here since clearing an Incoming message should not add a data object after resetting hash table.
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

                read_buf.receive_char(udp.read());

                count ++;

            }
            //read_buf.receive_char('\0'); // add null character so that extracted messages can be read as c strings. 
            read_buf.end_msg();
        }
    }
    return count;   
}

size_t WiFiPayload::read(){ // clears current read message. return size of first message in read_buf, and load into read_mes_buf

    //memset(read_mes_buf, '/0', MESSAGE_SIZE);

    in_data.clear(); // with this setup, in_data gets cleared even if no hash table exists. Make clear() virtual, with the Incoming_Data version only clearing if ready = true, and flipping ready to false after. 
    
    Serial.println("VALUE OF extract_message()");
        
    size_t y = read_buf.send_message(read_mes_buf);

    Serial.println(y);

    // extract crc

    char message_without_crc[1024];

    uint32_t claimed_crc = 0;

    size_t i = 0;

    while(read_mes_buf[i] != '{'){
        claimed_crc = claimed_crc + (read_mes_buf[i] - '0');
        if(read_mes_buf[i + 1] == '{'){
            break; //don't multiply by 10 after adding digit in the 10's place
        }
        claimed_crc = claimed_crc * 10;
        i++;
    }

    uint8_t message_from_dds[1024];

    Serial.println("message from DDS:");

     Serial.println(strlen(&read_mes_buf[i+1]));

    for(size_t j = 0; j < strlen(&read_mes_buf[i+1]); j++){
        message_from_dds[j] = read_mes_buf[i+1+j];
    }

    FastCRC32 CRC32;

    uint32_t calculated_crc = CRC32.crc32(message_from_dds, strlen(&read_mes_buf[i+1]));

    in_data.DATAroot = in_data.jsonBuffer.parseObject(&read_mes_buf[i+1], 50);

    if(!in_data.DATAroot.success()){
         Serial.println("PARSING FAILED");
    }

    

    //  char serialized_message[1024];

    //  in_data.DATAroot.printTo(serialized_message);

    //  Serial.println("differences below:");

    //  Serial.println("Serialized MEssage:");

    //  Serial.println(strlen(serialized_message));

     

    //  for(size_t k = 0; k <strlen(serialized_message); k++){
    //      if(serialized_message[k] == message_from_dds[k]){
    //          Serial.print("Different on line ");
    //          Serial.print(k);
    //          Serial.print(" serialized is ");
    //          Serial.print((int)serialized_message[k]);
    //          Serial.print(" message_from_dds is ");
    //          Serial.println(message_from_dds[k]);
    //      }
    //  }

    //  if(strcmp(serialized_message, message_from_dds)){
    //     Serial.println("Serialized Message is Different than non-serialized message!!!!!! line 164 WiFiPayload");
        
    //  }
    // //calculate crc

    // // HashPrint crc;
    // // in_data.DATAroot.printTo(crc);
    // // uint32_t calculated_crc = crc.hash();

    if(claimed_crc != calculated_crc){
        Serial.println("CRC DID NOT MATCH");
        Serial.println(claimed_crc);
        Serial.println(calculated_crc);
        return 0;
    }

    Serial.println("before create obj map line 101");

    if(in_data.DATAroot.success()){ // checks for valid JSON
        int x = in_data.DATAroot["data"]["b"];
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

    Serial.println("line 147 in heartbeat");

    StaticJsonBuffer<100> tempBuffer; // buffer on stack. Use DynamicJsonBuffer for buffer on heap.
    JsonObject& temp_root = tempBuffer.createObject(); // initialize root of JSON object. Memory freed when root goes out of scope.
    temp_root.set<String>("msg_type", "heartbeat");
    temp_root.set<String>("ip", myWiFi->localIP().toString());
    temp_root.set<String>("device_name", device_name);

    Serial.println("line 155 in heartbeat");

    HashPrint crc;
    temp_root.printTo(crc);
    uint32_t crc_hash = crc.hash();

    uint32_t copy = crc_hash;

    size_t crc_digits = 0;

    while(copy != 0){
        crc_digits++;
        copy = copy / 10;
    }

    Serial.println("line 161 in heartbeat");

    for(size_t i = 0; i < crc_digits; i++){
        heart_buf[(crc_digits-1)-i] = (crc_hash % 10) + '0'; // plus '0' to convert to ascii digits
        crc_hash = crc_hash / 10;
    }

    Serial.println("line 172 in heartbeat");

    Serial.println("line 174 in heartbeat");

    temp_root.printTo(&heart_buf[crc_digits], tempBuffer.size()); // convert from JSON object to c string

    Serial.println("HEARTBEAT BEING SENT TO DDS");

    Serial.println(heart_buf);

    //write to circ_buf
    write_buf.receive_out(heart_buf, tempBuffer.size() + 10); // no null characters in this message ever besides the terminating one hopefully
        //delay(10000);
    // }
    write_buf.send_out(udp, udpAddress, udpPort);

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
    Serial.print(myWiFi->disconnect(true));

    Serial.println("udp.begin()");
    Serial.println(udp.begin(udpPort));

    //register event handler
    myWiFi->onEvent(WiFiEvent);
  
    //Initiate connection
    myWiFi->begin(networkName, networkPswd); // password must be chars with ASCII values between 32-126 (decimal)

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