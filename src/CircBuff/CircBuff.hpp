#ifndef CircBuff_hpp
#define CircBuff_hpp

#include "Arduino.h"
#include <WiFiUdp.h> // for read()
#include "../Values.hpp" // for values namespace

class CircBuff{

    public:

        // constructor. Sets all indices of starts and ends to NULL
        CircBuff();

        // static array used as the circular buffer
        char circ_buf[values::CIRC_LENGTH];

        // confirms that a message of length value would fit in circ_buf. Returns false if not
        bool checkSize(size_t value);

        // adds the address of circ_buf[head] to starts
        void add_start(size_t head);

        // ads the address of circ_buf[tail] to ends
        void add_end(size_t head);

    protected:

        // array of char pointers to starts of all messages in circ_buf
        char* starts[values::CIRC_LENGTH/4];

        // array of char pointers to ends of all messages in circ_buf
        char* ends[values::CIRC_LENGTH/4];

        // indices correlate to a single message in starts and ends.
        // For example, starts[0] returns a pointer to the start of the first message inserted into circ_buf,
        // and ends[0] returns a pointer to the end of the first message inserted into circ_buf.

        // index of starts and ends where next incoming message will be inserted
        size_t add = 0; 

        // index of starts and ends for the next outgoing message to be read
        size_t rem = 0; 

        // space left in buffer
        size_t capacity = values::CIRC_LENGTH;

        // index of circ_buf where next incoming byte will be written into
        size_t head = 0;

        // index of circ_buf where next outgoing byte will be read from
        size_t tail = 0;
};

#endif