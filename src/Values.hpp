#ifndef Values_hpp
#define Values_hpp

#include "Arduino.h"

namespace values {

    const int MESSAGE_SIZE = 1024;

    const int MESSAGE_WITH_CRC_SIZE = MESSAGE_SIZE + 4;

    // msg_type + ip + device_name + "data" + crc
    const int DEAD_SPACE = 56; 

    // length of circular buffer. Configured to be able to store 4 messages at once
    const int CIRC_LENGTH = 4 * MESSAGE_WITH_CRC_SIZE;

    // smallest possible message is a 4 byte crc plus a 4 byte empty json object. 
    // Worst case is circ_buf is completely filled with these messages.
    const int STARTS_ENDS_SIZE = CIRC_LENGTH / 8; 

    // length of user input messages such as device_name
    const int NAME_SIZE = 30; 
    
}

#endif