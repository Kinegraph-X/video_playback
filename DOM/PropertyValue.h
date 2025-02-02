#pragma once 
#include "player_headers.h" // IWYU pragma: export

template <typename T>
struct PropertyValue {
    std::string name;
    mutable T value;
    mutable bool isDefault;
    
    PropertyValue(std::string propName, T defaultValue) 
        : name(propName), value(defaultValue), isDefault(true) {}
    
    void setValue(const T newValue) const {
        value = newValue;
        isDefault = false;
    }
    
    PropertyValue& operator=(const T& newValue) {
        setValue(newValue);
        return *this;
    }
};
