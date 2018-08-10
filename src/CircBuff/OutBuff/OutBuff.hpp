#ifndef OutBuff_hpp
#define OutBuff_hpp

#include "Arduino.h"
#include "../CircBuff.hpp"

class WiFiUDP; // forward decleration

class OutBuff : public CircBuff{

    public:
    
        // receives message to be inserted in circ_buf
        size_t push_outgoing_message(char* source_in, size_t size);

        // sends all messages contained in circ_buf
        size_t send_outgoing_messages(WiFiUDP& udp, const char* udpAddress, const int udpPort);

};

#endif