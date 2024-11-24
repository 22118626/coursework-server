#ifndef COURSEWORK_SERVER_RECORD_H
#define COURSEWORK_SERVER_RECORD_H

#include <iostream>
#include <utility>
#include <vector>
#include <memory>
#include <variant>
#include <fstream>
#include <cstring>
#include <iomanip>


/*struct BaseType {
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
};*/

struct FieldData {
    std::string name;
    uint8_t type;
    uint16_t length;

};

struct BaseType {
    std::string name;
    BaseType(std::string name) : name(std::move(name)) {}
    virtual ~BaseType() = default;

    virtual std::variant<int16_t, uint16_t, int32_t, uint32_t, std::string, bool, std::pair<std::string, uint32_t>> getValue() const = 0;
};

template <typename T>
struct Field : public BaseType {
    T value;
    Field(std::string name, T val) : BaseType(std::move(name)), value(val) {}

    std::variant<int16_t, uint16_t, int32_t, uint32_t, std::string, bool, std::pair<std::string, uint32_t>> getValue() const override {
        return value;
    }
};

struct StringField : Field<std::string> {
    uint16_t dataLength;

    StringField(std::string name, std::string val, uint16_t dataLength)
            : Field<std::string>(std::move(name), std::move(val)), dataLength(dataLength) {}

    std::variant<int16_t, uint16_t, int32_t, uint32_t, std::string, bool, std::pair<std::string, uint32_t>> getValue() const override {
        return value;
    }
};

struct ReferenceField : Field<std::pair<std::string, uint32_t>> {
    std::string refName;
    uint32_t primaryKeyValue;

    ReferenceField(std::string name, std::string refName, uint32_t primaryKeyValue)
            : Field<std::pair<std::string, uint32_t>>(std::move(name), {std::move(refName), primaryKeyValue}),
              refName(std::move(refName)), primaryKeyValue(primaryKeyValue) {}

    std::variant<int16_t, uint16_t, int32_t, uint32_t, std::string, bool, std::pair<std::string, uint32_t>> getValue() const override {
        return value;
    }
};

using Int16Field = Field<int16_t>;
using UInt16Field = Field<uint16_t>;
using Int32Field = Field<int32_t>;
using UInt32Field = Field<uint32_t>;
using BoolField = Field<bool>;


struct Record {
    std::vector<uint8_t> data;

    std::ostream& operator<<(std::ostream &os) {
        for (const auto& byte : this->data) {
            os << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
        }
        return os;
    }

    template <typename T>
    T getFieldData(size_t offset) const {
        T value;
        std::memcpy(&value, &data[offset], sizeof(T));
        return value;
    }
    template <typename T>
    Record& modifyField(Record* record, T dataToChange, size_t offset) {
        std::memcpy(record->data[offset] , dataToChange, sizeof(T));
        return *record;
    }
    template <typename T>
    Record& appendField(Record* record, const T& dataChange, unsigned int length) {
        record->data.resize((record->data.size()) + length);
        std::memcpy(record->data.data() + record->data.size() - length, &dataChange, length);
        return *record;
    }
    Record& appendStringField(Record* record, const std::string& dataChange, unsigned int length) {
        record->data.resize((record->data.size()) + length);
        for (size_t i = 0; i < dataChange.size(); ++i) {
            record->data[record->data.size() - length + i] = static_cast<uint8_t>(dataChange[i]);
        }
        for (size_t i = dataChange.size(); i < length; ++i) {
            record->data[record->data.size() - length + i] = 0x00;
        }
        return *record;
    }
};


class RecordClass {
public:
    Record* structure;
    std::vector<std::shared_ptr<BaseType>> record; //shared_ptr for polymorphism
    std::vector<int> typeArray;

    virtual ~RecordClass() = default;

    static std::string printRecords(Record recordObj, std::vector<int> typesArray);
    int fieldName(const std::string &name);

    template <typename T>
    auto returnvalueFromindex(int index) {
        for (const auto& item : record) {
            std::cout << typeid(*item).name() << std::endl; // Print the type of each item in record
        }
        std::cout << "Template type T: " << typeid(Field<T>).name() << std::endl;
        std::cout << "record type: " << typeid(std::dynamic_pointer_cast<Field<T>&>(record.at(index))).name() << std::endl;
        auto field = std::dynamic_pointer_cast<Field<T>>(record.at(index));
        std::cout << "Field at index " << index << ": " << field << std::endl;
        if(field) return field->value;
        else throw std::runtime_error("Invalid type. index: "+std::to_string(index));
    }
};

#endif //COURSEWORK_SERVER_RECORD_H
