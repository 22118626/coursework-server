#ifndef COURSEWORK_SERVER_RECORD_H
#define COURSEWORK_SERVER_RECORD_H

#include <iostream>
#include <utility>
#include <vector>
#include <memory>



struct BaseType {
    std::string name;
    BaseType(std::string name) : name(std::move(name)) {}
    virtual ~BaseType() = default;
};

struct Int16Field : BaseType {
    int16_t value;
    Int16Field(std::string name, int16_t val) : BaseType(std::move(name)), value(val) {}
};
struct UInt16Field : BaseType {
    uint16_t value;
    UInt16Field(std::string name, uint16_t val) : BaseType(std::move(name)), value(val) {}
};
struct Int32Field : BaseType {
    int32_t value;
    Int32Field(std::string name, int32_t val) : BaseType(std::move(name)), value(val) {}
};
struct UInt32Field : BaseType {
    uint32_t value;
    UInt32Field(std::string name, uint32_t val) : BaseType(std::move(name)), value(val) {}
};
struct StringField : BaseType {
    std::string value;
    uint16_t dataLength;
    StringField(std::string name, std::string val, uint16_t dataLength) : BaseType(std::move(name)), value(std::move(val)), dataLength(dataLength) {}
};
struct ReferenceField : BaseType {
    std::string value;
    uint32_t primaryKeyValue;
    ReferenceField(std::string name, std::string val, uint32_t val2) : BaseType(std::move(name)), value(std::move(val)), primaryKeyValue(val2) {}
};
struct BoolField : BaseType {
    bool value;
    BoolField(std::string name, bool val) : BaseType(std::move(name)), value(val) {}
};


class Record {
public:
    virtual ~Record() = default;

    std::vector<std::shared_ptr<BaseType>> record; //shared_ptr for polymorphism
    static std::string printRecord(Record recordObj, std::vector<int> typesArray);
};

#endif //COURSEWORK_SERVER_RECORD_H
