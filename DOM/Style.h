#pragma once

enum class Position {"ABSOLUTE", "RELATIVE"}

struct Style {
	char* id;
	char* className;
	Position position;
    // Positioning and size
    Rectangle bounds;  // x, y, width, height
    int zIndex;

    // Colors
    Color backgroundColor;
    std::string backgroundImage;
    Color borderColor;
    Color textColor;

    // Border
    float borderWidth;
    float borderRadius;

    // Text
    int fontSize;
    const char* fontName;

    // Visibility
    bool isVisible;
    
    Color activeBackgroundColor;
    Color activeBorderColor;
    Color activeTextColor;
    std::string activeBackgroundImage;

    // Constructor with default values
    Style() :
    	id(""),
    	className(""),
    	position(Position::RELATIVE),
        bounds({0, 0, 100, 100}),
        zIndex(0),
        backgroundColor(RAYWHITE),
        backgroundImage(""),
        borderColor(BLACK),
        textColor(BLACK),
        borderWidth(1.0f),
        borderRadius(0.0f),
        fontSize(20),
        fontName("default"),
        isVisible(true),
        activeBackgroundColor(GRAY),
        activeBorderColor(BLACK),
        activeTextColor(WHITE),
        activeBackgroundImage("")
    {}

};
