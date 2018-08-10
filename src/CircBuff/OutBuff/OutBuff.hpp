#ifndef OutBuff_hpp
#define OutBuff_hpp

#include "Arduino.h"
#include "../CircBuff.hpp"

class WiFiUDP; // forward decleration

class OutBuff : public CircBuff{

    public:
    
        // receives message to be inserted in circ_buf
        size_t receive_out(char* source_in, size_t size);

        // sends all messages
        size_t send_out(WiFiUDP& udp, const char* udpAddress, const int udpPort);

};

#endif