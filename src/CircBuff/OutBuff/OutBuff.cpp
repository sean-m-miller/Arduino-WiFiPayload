#include "Arduino.h"
#include "OutBuff.hpp"

size_t OutBuff::push_outgoing_message(char* mes_buf, size_t mes_length){

    // if message fits in circ_buf, add to circ_buf and update fields
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

size_t OutBuff::send_outgoing_messages(WiFiUDP& udp, const char* udpAddress, const int udpPort){

    // number of messages sent
    size_t wrote = 0;

    // flushes circ_buf by sending all messages
    while(head != tail){
        udp.beginPacket(udpAddress,udpPort);

        // no wrap around
        if(ends[rem] > starts[rem]){ 
            for(size_t i = 0; i < (ends[rem] - starts[rem]); i++){
                udp.write(circ_buf[tail]);
                tail = (tail + 1) % values::CIRC_LENGTH;
                capacity++;
            }
        }

        //wrappity roo
        else{ 
            for(size_t i = 0; i < ((ends[rem] - circ_buf) + ((circ_buf + values::CIRC_LENGTH) - starts[rem])); i++){ // draw a picture... This is correct
                udp.write(circ_buf[tail]);
                tail = (tail + 1) % values::CIRC_LENGTH;
                capacity++;
            }
        }

        // incriment by 1 (message sent) or 0 (message not sent)
        wrote = wrote + udp.endPacket();

        // incriment rem. In this case, we do not need to account for wrap around (rem = (rem + 1) % values::STARTS_ENDS_SIZE).
        // This is because with each send_out(), all messages get sent, so add and rem get reset to 0 before more messages added
        rem++;
    }
    add = 0;
    rem = 0;
    return wrote;
}

