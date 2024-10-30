#ifndef COURSEWORK_SERVER_RECORD_H
#define COURSEWORK_SERVER_RECORD_H

#include <iostream>

class Record {
public:
    virtual ~Record() = default;
    virtual void print() const = 0;
};

#endif //COURSEWORK_SERVER_RECORD_H
