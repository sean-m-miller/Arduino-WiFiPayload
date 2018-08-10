#ifndef InBuff_hpp
#define InBuff_hpp

#include "Arduino.h"
#include "../CircBuff.hpp"

class InBuff : public CircBuff{

    public:

        // function to be called before receive_char()
        size_t start_msg();

        // add a single char c into circ_buf.
        size_t push_incoming_char(char c);

        // function to be called after receive_char()
        size_t end_msg();

        // pops the next message off of circ_buf, and copies it into destination
        size_t pop_incoming_message(char* destination);
};

#endif