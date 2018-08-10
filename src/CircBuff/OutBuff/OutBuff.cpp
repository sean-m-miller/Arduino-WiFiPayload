#include "Arduino.h"
#include "OutBuff.hpp"

size_t OutBuff::receive_out(char* mes_buf, size_t mes_length){
    if(checkSize(mes_length)){
        add_start(head);
        for(size_t i = 0; i < mes_length; i++){
            circ_buf[head] = mes_buf[i];
            head = (head + 1) % values::CIRC_LENGTH;
            capacity--;
        }
        add_end(head); 
        return 1;
    }
    return 0;
}

size_t OutBuff::send_out(WiFiUDP& udp, const char* udpAddress, const int udpPort){ // process messages and send accross udp
    size_t wrote = 0; // did we write something?
    while(head != tail){
        udp.beginPacket(udpAddress,udpPort);
        if(ends[rem] > starts[rem]){ // no wrap around
            for(size_t i = 0; i < (ends[rem] - starts[rem]); i++){
                udp.write(circ_buf[tail]);
                tail = (tail + 1) % values::CIRC_LENGTH;
                capacity++;
            }
        }
        else{ //wrappity roo
            for(size_t i = 0; i < ((ends[rem] - circ_buf) + ((circ_buf + values::CIRC_LENGTH) - starts[rem])); i++){ // draw a picture... This is correct
                udp.write(circ_buf[tail]);
                tail = (tail + 1) % values::CIRC_LENGTH;
                capacity++;
            }
        }
        udp.write('\0'); // ensure message to DDS is cstring
        wrote = wrote + udp.endPacket(); // number of messages wrote (NOT BYTES!)
        Serial.print("BYTES WRITTEN: ");
        Serial.print(wrote);
        rem++;
    }
    add = 0;
    rem = 0;
    return wrote;
}

