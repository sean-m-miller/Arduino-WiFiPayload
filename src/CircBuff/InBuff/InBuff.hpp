#ifndef InBuff_hpp
#define InBuff_hpp

#include "Arduino.h"
#include "../CircBuff.hpp"

class InBuff : public CircBuff{

    public:

        size_t receive_char(char c);

        size_t send_message(char* destination);

        size_t start_msg();

        size_t end_msg();

};

#endif