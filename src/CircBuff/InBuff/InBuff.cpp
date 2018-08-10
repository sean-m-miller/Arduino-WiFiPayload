#include "Arduino.h"
#include "InBuff.hpp"

size_t InBuff::push_incoming_char(char c){

    // insert char into circ_buf and update class fields
    if(capacity){
        circ_buf[head] = c;
        head = (head + 1) % values::CIRC_LENGTH;
        capacity--;
        return 1;
    }
    return 0;
}

size_t InBuff::pop_incoming_message(char* destination){

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
        rem = (rem + 1) % (values::STARTS_ENDS_SIZE); //in read_buf case, rem and add can wrap around
    }
    return wrote;
}

size_t InBuff::start_msg(){

    // wrapper for beginning a new entry. A call to start_msg requires a call from end_msg() after all receive_char() calls are finished.
    if(capacity){
        add_start(head);
        return 1;
    }
    return 0;
}

size_t InBuff::end_msg(){
    add_end(head);
    return 1;
}