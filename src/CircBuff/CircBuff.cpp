#include "Arduino.h"
#include "CircBuff.hpp"

CircBuff::CircBuff(){
    for(int i  = 0; i < values::STARTS_ENDS_SIZE; i++){
        starts[i] = NULL;
        ends[i] = NULL;
    }
}

bool CircBuff::checkSize(size_t value){

    // will this incoming message of size value fit?
    return (value <= capacity && value <= (values::MESSAGE_WITH_CRC_SIZE));
}

void CircBuff::add_start(size_t head){
    char* temp = circ_buf + head;
    starts[add] = temp;
}

void CircBuff::add_end(size_t head){
    char* temp = circ_buf + head;
    ends[add] = temp;

    //incriment so that next add is at next index of starts and ends
    add = (add + 1) % (values::STARTS_ENDS_SIZE); 
}

// CODE BELOW: used for debugging Circbuff

// do not include flag in message
        //capacity = capacity - (mes_length + 1); // +1 for flag
        // for(int i = 0; i < 4096; i++){
        //     Serial.print(circ_buf[i]);
        //     Serial.print(" at index ");
        //     Serial.print(i);
        //     Serial.println("");
        // }
        // Serial.println("HEAD:");
        // Serial.println((int)head);
        // Serial.println("TAIL:");
        // Serial.println((int)tail);