#include "Arduino.h"
#include "InBuff.hpp"

size_t InBuff::receive_char(char c){
    if(capacity > 4){
        circ_buf[head] = c;
        head = (head + 1) % values::CIRC_LENGTH;
        capacity--;
        return 1;
    }
    return 0;
}

size_t InBuff::send_message(char* destination){

    // number of bytes we write
    size_t wrote = 0; 

    // if any messages to be extracted
    if(capacity < values::CIRC_LENGTH){

        // no wrap around
        if(ends[rem] > starts[rem]){
            for(size_t i = 0; i < (ends[rem] - starts[rem]); i++){
                destination[i] = circ_buf[tail];
                tail = (tail + 1) % values::CIRC_LENGTH;
                capacity++;
                wrote++;
            }
        }

        // wrappity roo
        else{ 
            for(size_t i = 0; i < ((ends[rem] - circ_buf) + ((circ_buf + values::CIRC_LENGTH) - starts[rem])); i++){ // draw a picture... This is correct
                destination[i] = circ_buf[tail];
                tail = (tail + 1) % values::CIRC_LENGTH;
                capacity++;
                wrote++;
            }
        }
        rem = (rem + 1) % (values::CIRC_LENGTH/4); //in read_buf case, rem and add can wrap around
    }
    return wrote;
}

size_t InBuff::start_msg(){
    if(capacity > 4){
        add_start(head);
        return 1;
    }
    return 0;
}

size_t InBuff::end_msg(){
    add_end(head);
    return 1;
}