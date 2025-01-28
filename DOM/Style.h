#pragma once

enum class Position {Absolute, Relative};

struct Style {
	char* id;
	char* className;
	Position position;
    // Positioning and size
    RaylibRectangle bounds;  // x, y, width, height
    int zIndex;

    // Colors
    RaylibColor backgroundColor;
    std::string backgroundImage;
    RaylibColor borderColor;
    RaylibColor textColor;

    // Border
    float borderWidth;
    float borderRadius;

    // Text
    int fontSize;
    const char* fontName;

    // Visibility
    bool isVisible;
    
    RaylibColor activeBackgroundColor;
    RaylibColor activeBorderColor;
    RaylibColor activeTextColor;
    std::string activeBackgroundImage;

    // Constructor with default values
    Style() :
    	id(""),
    	className(""),
    	position(Position::Relative),
        bounds({0, 0, 100, 100}),
        zIndex(0),
        backgroundColor(RAYLIB_RAYWHITE),
        backgroundImage(""),
        borderColor(RAYLIB_BLACK),
        textColor(RAYLIB_BLACK),
        borderWidth(1.0f),
        borderRadius(0.0f),
        fontSize(20),
        fontName("default"),
        isVisible(true),
        activeBackgroundColor(RAYLIB_GRAY),
        activeBorderColor(RAYLIB_BLACK),
        activeTextColor(RAYLIB_WHITE),
        activeBackgroundImage("")
    {}

};
