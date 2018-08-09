#ifndef Values_hpp
#define Values_hpp

#include "Arduino.h"

namespace values {

    const int MESSAGE_SIZE = 1024;

    const int DEAD_SPACE = 56; // msg_type + ip + device_name + "data" + crc

    const int SIZE_TO_ADD_NEW = 44; // adding a nested object or array is larger than the sum of its bytes

    const int CIRC_LENGTH = 4096;

    const int NAME_SIZE = 30; // length of user input messages such as device_name
    
}

#endif