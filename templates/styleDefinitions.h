#pragma once
#include "../DOM/StyleManager.h"

void createStyles(StyleManager& styleManager) {

	// Define styles for IDs
	Style mainContainerStyle;
	mainContainerStyle.backgroundColor = RAYLIB_BLANK;
	mainContainerStyle.backgroundImage = "assets/videoplayer_remote.png";
	mainContainerStyle.bounds = {0, 505, 900, 144};
	styleManager.addIdStyle("main_container", mainContainerStyle);
	
	// Define styles for classes
	Style buttonStyle;
	buttonStyle.backgroundColor = RAYLIB_BLANK;
	buttonStyle.backgroundImage = "assets/videoplayer_play_button.png";
	buttonStyle.activeBackgroundImage = "assets/videoplayer_play_button_pressed.png";
	buttonStyle.textColor = RAYLIB_BLACK;
	buttonStyle.bounds = {387, 23, 113, 113};
	buttonStyle.fontSize = 20;
	styleManager.addClassStyle("button", buttonStyle);
	
//	Style playButtonStyle = buttonStyle;
//	playButtonStyle.backgroundColor = RAYLIB_GREEN;
//	styleManager.addClassStyle("play_button", playButtonStyle);
//	
//	Style stopButtonStyle = buttonStyle;
//	stopButtonStyle.backgroundColor = RAYLIB_RED;
//	styleManager.addClassStyle("stop_button", stopButtonStyle);
}