//
// Created by ener9 on 11/11/2024.
//

#include "Record.h"

std::string Record::printRecords(Record recordObj, std::vector<int> typesArray) {
    std::string returnString = "";
    for(int i = 0 ; i < recordObj.record.size() ; i++) {
        std::cout << std::to_string(i) << "\t" << recordObj.record.at(i) << "\t" << typesArray[i] << std::endl;
        switch (typesArray[i]) {
            case 3:
                returnString += recordObj.record.at(i)->name + " of type: String of max size :" +
                        std::to_string(std::dynamic_pointer_cast<StringField>(recordObj.record.at(i))->dataLength) + "\n";
                break;
            case 4:
                returnString += recordObj.record.at(i)->name + " of type: Reference " +"\n";
                break;
            default:
                returnString += recordObj.record.at(i)->name + " of type: " + std::to_string(typesArray[i]) +"\n";
                break;
        }
    }
    return returnString;
}

/*auto Record::returnvalueFromindex(int index) {
    return this->record.at(0)->value;
    std::shared_ptr<BaseType> rtrnval = std::shared_ptr<BaseType>(this->record.at(index));
}*/


int Record::fieldName(const std::string& name) {
    for (int i = 0; i < this->record.size(); i++) {
        if (this->record.at(i)->name == name) {
            return i; // return the index of the field
        }
    }
    return -1; // Return -1 if not found
}