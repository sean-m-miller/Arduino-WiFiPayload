#include "Arduino.h"
#include "CircBuff.hpp"

CircBuff::CircBuff(){
    for(int i  = 0; i < values::CIRC_LENGTH/4; i++){
        starts[i] = NULL;
        ends[i] = NULL;
    }
}

bool CircBuff::checkSize(size_t value){
    return (value < capacity && value < values::MESSAGE_SIZE); // leave one index to be safe? double check to make sure > -1 wouldn't break something
}

void CircBuff::add_start(size_t head){
    char* temp = circ_buf + head;
    starts[add] = temp;
}

void CircBuff::add_end(size_t head){
    char* temp = circ_buf + head;
    ends[add] = temp;
    add = (add + 1) % (values::CIRC_LENGTH/4); //incriment so that next add is at next index of starts and ends
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