#pragma once
#include "player_headers.h"
#include "Style.h"

struct ComputedStyle {
    PropertyValue<Position> position;
    PropertyValue<RaylibRectangle> bounds;
    PropertyValue<int> zIndex;
    PropertyValue<RaylibColor> backgroundColor;
    PropertyValue<std::string> backgroundImage;
    PropertyValue<RaylibColor> borderColor;
    PropertyValue<RaylibColor> textColor;
    PropertyValue<float> borderWidth;
    PropertyValue<int> borderRadius;
    PropertyValue<int> fontSize;
    PropertyValue<std::string> fontName;
    PropertyValue<bool> isVisible;
    
    ComputedStyle() :
    	position(DefaultStyle::position),
        bounds(DefaultStyle::bounds),
        zIndex(DefaultStyle::zIndex),
        backgroundColor(DefaultStyle::backgroundColor),
        backgroundImage(DefaultStyle::backgroundImage),
        borderColor(DefaultStyle::borderColor),
        textColor(DefaultStyle::textColor),
        borderWidth(DefaultStyle::borderWidth),
        borderRadius(DefaultStyle::borderRadius),
        fontSize(DefaultStyle::fontSize),
        fontName(DefaultStyle::fontName),
        isVisible(DefaultStyle::isVisible)
    {
		logger(LogLevel::DEBUG, "random computedStyle prop name : " + this->position.name);
	};
    
    static constexpr size_t NUMBER_OF_PROPERTIES = 12;
    
    class Iterator {
	    private:
        ComputedStyle* computedStyle;
        size_t index;
	
	    public:
        Iterator(ComputedStyle* s, size_t i) : computedStyle(s), index(i) {}
		
		using PropertyVariant = std::variant<
		    std::reference_wrapper<PropertyValue<Position>>,
		    std::reference_wrapper<PropertyValue<RaylibRectangle>>,
		    std::reference_wrapper<PropertyValue<RaylibColor>>,
		    std::reference_wrapper<PropertyValue<int>>,
		    std::reference_wrapper<PropertyValue<float>>,
		    std::reference_wrapper<PropertyValue<bool>>,
		    std::reference_wrapper<PropertyValue<std::string>>
		>;

		const PropertyVariant getPropByName() {
            // Return the appropriate property based on index
            switch(index) {
                case 0: return computedStyle->position;
                case 1: return computedStyle->bounds;
                case 2: return computedStyle->zIndex;
                case 3: return computedStyle->backgroundColor;
                case 4: return computedStyle->backgroundImage;
                case 5: return computedStyle->borderColor;
                case 6: return computedStyle->textColor;
                case 7: return computedStyle->borderWidth;
                case 8: return computedStyle->borderRadius;
                case 9: return computedStyle->fontSize;
                case 10: return computedStyle->fontName;
                case 11: return computedStyle->isVisible;
                
                default: 
                	logger(LogLevel::ERR, "Invalid index in ComputedStyle iterator");
//                	return DefaultStyle::error;
            }
        }
        
        const PropertyVariant operator*() {
			return getPropByName();
		}

        Iterator& operator++() {
            ++index;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }
    };
	
	Iterator begin() { return Iterator(this, 0); };
    Iterator end() { return Iterator(this, NUMBER_OF_PROPERTIES); };
};