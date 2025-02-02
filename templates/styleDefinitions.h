#pragma once
#include "../DOM/StyleManager.h"

void createStyles(StyleManager& styleManager) {

	// Define styles for IDs
	Style mainContainerStyle;
	mainContainerStyle.backgroundColor = RAYLIB_RAYWHITE;
	mainContainerStyle.bounds = {5, 360, 560, 70};
	styleManager.addIdStyle("main_container", mainContainerStyle);
	
	// Define styles for classes
//	Style buttonStyle;
//	buttonStyle.backgroundColor = RAYLIB_LIGHTGRAY;
//	buttonStyle.textColor = RAYLIB_BLACK;
//	buttonStyle.bounds = {0, 0, 100, 50};
//	buttonStyle.fontSize = 20;
//	styleManager.addClassStyle("button", buttonStyle);
//	
//	Style playButtonStyle = buttonStyle;
//	playButtonStyle.backgroundColor = RAYLIB_GREEN;
//	styleManager.addClassStyle("play_button", playButtonStyle);
//	
//	Style stopButtonStyle = buttonStyle;
//	stopButtonStyle.backgroundColor = RAYLIB_RED;
//	styleManager.addClassStyle("stop_button", stopButtonStyle);
}