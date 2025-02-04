#pragma once
#include "PropertyValue.h"

enum class Position {Absolute, Relative};

using PropertyVariant = std::variant<
    std::reference_wrapper<PropertyValue<Position>>,
    std::reference_wrapper<PropertyValue<RaylibRectangle>>,
    std::reference_wrapper<PropertyValue<RaylibColor>>,
    std::reference_wrapper<PropertyValue<int>>,
    std::reference_wrapper<PropertyValue<float>>,
    std::reference_wrapper<PropertyValue<bool>>,
    std::reference_wrapper<PropertyValue<std::string>>
>;    

const std::vector<std::string> INHERITED_PROPERTIES = {
    "textColor",
    "fontName",
    "fontSize",
    "opacity",
    "isVisible"
};

// These are populated in the cpp file
struct DefaultStyle {
    static const PropertyValue<Position> position;
    static const PropertyValue<RaylibRectangle> bounds;
    static const PropertyValue<int> zIndex;
    static const PropertyValue<RaylibColor> backgroundColor;
    static const PropertyValue<std::string> backgroundImage;
    static const PropertyValue<RaylibColor> borderColor;
    static const PropertyValue<RaylibColor> textColor;
    static const PropertyValue<float> borderWidth;
    static const PropertyValue<int> borderRadius;
    static const PropertyValue<int> fontSize;
    static const PropertyValue<std::string> fontName;
    static const PropertyValue<bool> isVisible;
    static const PropertyValue<std::string> error;
};


struct Style {
    std::string id;
    std::string className;
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
    
    // Active and hover states
    PropertyValue<RaylibRectangle> activeBounds;
    PropertyValue<RaylibColor> activeBackgroundColor;
    PropertyValue<float> activeBorderWidth;
    PropertyValue<RaylibColor> activeBorderColor;
    PropertyValue<RaylibColor> activeTextColor;
    PropertyValue<std::string> activeBackgroundImage;
    
    PropertyValue<RaylibRectangle> hoverBounds;
    PropertyValue<RaylibColor> hoverBackgroundColor;
    PropertyValue<float> hoverBorderWidth;
    PropertyValue<RaylibColor> hoverBorderColor;
    PropertyValue<RaylibColor> hoverTextColor;
    PropertyValue<std::string> hoverBackgroundImage;

    Style() :
        id(""),
        className(""),
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
        isVisible(DefaultStyle::isVisible),
        
        activeBounds(DefaultStyle::bounds),
        activeBackgroundColor(DefaultStyle::backgroundColor),
        activeBorderWidth(DefaultStyle::borderWidth),
        activeBorderColor(DefaultStyle::borderColor),
        activeTextColor(DefaultStyle::textColor),
        activeBackgroundImage(DefaultStyle::backgroundImage),
        
        hoverBounds(DefaultStyle::bounds),
        hoverBackgroundColor(DefaultStyle::backgroundColor),
        hoverBorderWidth(DefaultStyle::borderWidth),
        hoverBorderColor(DefaultStyle::borderColor),
        hoverTextColor(DefaultStyle::textColor),
        hoverBackgroundImage(DefaultStyle::backgroundImage)
    {};
    
//	using PropertyVariant = std::variant<
//	    std::reference_wrapper<PropertyValue<Position>>,
//	    std::reference_wrapper<PropertyValue<RaylibRectangle>>,
//	    std::reference_wrapper<PropertyValue<RaylibColor>>,
//	    std::reference_wrapper<PropertyValue<int>>,
//	    std::reference_wrapper<PropertyValue<float>>,
//	    std::reference_wrapper<PropertyValue<bool>>,
//	    std::reference_wrapper<PropertyValue<std::string>>
//	>;    
    const PropertyVariant operator[](const std::string& propName) {
	    if (propName == "position") return position;
	    if (propName == "bounds") return bounds;
	    if (propName == "zIndex") return zIndex;
	    if (propName == "backgroundColor") return backgroundColor;
	    if (propName == "backgroundImage") return backgroundImage;
	    if (propName == "borderColor") return borderColor;
	    if (propName == "textColor") return textColor;
	    if (propName == "borderWidth") return borderWidth;
	    if (propName == "borderRadius") return borderRadius;
	    if (propName == "fontSize") return fontSize;
	    if (propName == "fontName") return fontName;
	    if (propName == "isVisible") return isVisible;
	    
	    if (propName == "activeBounds") return position;
	    if (propName == "activeBackgroundColor") return activeBackgroundColor;
	    if (propName == "activeBorderWidth") return activeBorderWidth;
	    if (propName == "activeBorderColor") return activeBorderColor;
	    if (propName == "activeTextColor") return activeTextColor;
	    if (propName == "activeBackgroundImage") return activeBackgroundImage;
	    
	    if (propName == "hoverBounds") return hoverBounds;
	    if (propName == "hoverBackgroundColor") return hoverBackgroundColor;
	    if (propName == "hoverBorderWidth") return hoverBorderWidth;
	    if (propName == "hoverBorderColor") return hoverBorderColor;
	    if (propName == "hoverTextColor") return hoverTextColor;
	    if (propName == "hoverBackgroundImage")  return hoverBackgroundImage;
	    
	    logger(LogLevel::ERR, "out_of_range : Property not found in Style instance : " + propName);
	};
    
    static constexpr size_t NUMBER_OF_PROPERTIES = 24;
    
    class Iterator {
	    private:
        Style* style;
        size_t index;
	
	    public:
        Iterator(Style* s, size_t i) : style(s), index(i) {}
		
		const PropertyVariant operator*() {
//			static PropertyVariant errorValue;
            // Return the appropriate property based on index
            switch(index) {
                case 0: return style->position;
                case 1: return style->bounds;
                case 2: return style->zIndex;
                case 3: return style->backgroundColor;
                case 4: return style->backgroundImage;
                case 5: return style->borderColor;
                case 6: return style->textColor;
                case 7: return style->borderWidth;
                case 8: return style->borderRadius;
                case 9: return style->fontSize;
                case 10: return style->fontName;
                case 11: return style->isVisible;
                
                case 12: return style->activeBounds;
			    case 13: return style->activeBackgroundColor;
			    case 14: return style->activeBorderWidth;
			    case 15: return style->activeBorderColor;
			    case 16: return style->activeTextColor;
			    case 17: return style->activeBackgroundImage;
			    
			    case 18: return style->hoverBounds;
			    case 19: return style->hoverBackgroundColor;
			    case 20: return style->hoverBorderWidth;
			    case 21: return style->hoverBorderColor;
			    case 22: return style->hoverTextColor;
			    case 23: return style->hoverBackgroundImage;
                
                default: logger(LogLevel::ERR, "Invalid index in Style iterator");
            }
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

