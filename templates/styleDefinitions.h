#pragma once
#include "../DOM/StyleManager.h"

void createStyles(StyleManager& styleManager) {

	// Define styles for IDs
	Style mainContainerStyle;
	mainContainerStyle.backgroundColor = RAYLIB_BLANK;
	mainContainerStyle.backgroundImage = "assets/videoplayer_remote.png";
//	mainContainerStyle.borderColor = RAYLIB_GRAY;
//	mainContainerStyle.borderWidth = 2;
	mainContainerStyle.bounds = {0, 505, 900, 144};
	styleManager.addIdStyle("main_container", mainContainerStyle);
	
	// Define styles for classes
	Style buttonStyle;
	buttonStyle.backgroundColor = RAYLIB_BLANK;
	buttonStyle.backgroundImage = "assets/videoplayer_button.png";
	buttonStyle.activeBackgroundImage = "assets/videoplayer_button_pressed.png";
//	buttonStyle.borderColor = RAYLIB_GRAY;
//	buttonStyle.borderWidth = 2;
	buttonStyle.textColor = RAYLIB_BLACK;
	buttonStyle.bounds = {18, 18, 57, 57};
	buttonStyle.fontSize = 20;
	styleManager.addClassStyle("button", buttonStyle);
	
	Style playButtonStyle = buttonStyle;
	playButtonStyle.backgroundImage = "assets/videoplayer_play_button.png";
	playButtonStyle.activeBackgroundImage = "assets/videoplayer_play_button_pressed.png";
	playButtonStyle.bounds = {418, 18, 57, 57};
	styleManager.addClassStyle("play_button", playButtonStyle);
	
	Style pauseButtonStyle = buttonStyle;
	pauseButtonStyle.backgroundImage = "assets/videoplayer_pause_button.png";
	pauseButtonStyle.activeBackgroundImage = "assets/videoplayer_pause_button_pressed.png";
	pauseButtonStyle.bounds = {418, 18, 57, 57};
	pauseButtonStyle.zIndex = 2;
	pauseButtonStyle.isVisible = false;
	styleManager.addClassStyle("pause_button", pauseButtonStyle);

	Style plusFiveButtonStyle = buttonStyle;
	plusFiveButtonStyle.backgroundImage = "assets/videoplayer_plus_five_button.png";
	plusFiveButtonStyle.activeBackgroundImage = "assets/videoplayer_plus_five_button_pressed.png";
	plusFiveButtonStyle.bounds = {501, 18, 57, 57};
	styleManager.addClassStyle("plus_five_button", plusFiveButtonStyle);
	
	Style minusFiveButtonStyle = buttonStyle;
	minusFiveButtonStyle.backgroundImage = "assets/videoplayer_minus_five_button.png";
	minusFiveButtonStyle.activeBackgroundImage = "assets/videoplayer_minus_five_button_pressed.png";
	minusFiveButtonStyle.bounds = {332, 18, 57, 57};
	styleManager.addClassStyle("minus_five_button", minusFiveButtonStyle);
	
	Style stopButtonStyle = buttonStyle;
	stopButtonStyle.backgroundImage = "assets/videoplayer_stop_button.png";
	stopButtonStyle.activeBackgroundImage = "assets/videoplayer_stop_button_pressed.png";
	stopButtonStyle.bounds = {584, 18, 57, 57};
	styleManager.addClassStyle("stop_button", stopButtonStyle);
	
	Style loadButtonStyle = buttonStyle;
	loadButtonStyle.backgroundImage = "assets/videoplayer_load_button.png";
	loadButtonStyle.activeBackgroundImage = "assets/videoplayer_load_button_pressed.png";
	loadButtonStyle.bounds = {151, 18, 57, 57};
	styleManager.addClassStyle("load_button", loadButtonStyle);
	
	Style handButtonStyle = buttonStyle;
	handButtonStyle.backgroundImage = "assets/videoplayer_hand_button.png";
	handButtonStyle.activeBackgroundImage = "assets/videoplayer_hand_button.png";
	handButtonStyle.bounds = {820, 18, 57, 57};
	styleManager.addClassStyle("hand_button", handButtonStyle);
	
	Style shutdownButtonStyle = buttonStyle;
	shutdownButtonStyle.backgroundImage = "assets/videoplayer_shutdown_button.png";
	shutdownButtonStyle.activeBackgroundImage = "assets/videoplayer_shutdown_button_pressed.png";
	shutdownButtonStyle.bounds = {23, 18, 57, 57};
	styleManager.addClassStyle("shutdown_button", shutdownButtonStyle);
	
	Style fullscreenButtonStyle;
	fullscreenButtonStyle.backgroundImage = "assets/videoplayer_fullscreen_button.png";
	fullscreenButtonStyle.activeBackgroundImage = "assets/videoplayer_fullscreen_button.png";
//	fullscreenButtonStyle.borderColor = RAYLIB_GRAY;
//	fullscreenButtonStyle.borderWidth = 2;
	fullscreenButtonStyle.bounds = {831, 18, 51, 51};
	styleManager.addClassStyle("fullscreen_button", fullscreenButtonStyle);
	
	Style audioSliderButtonStyle;
	audioSliderButtonStyle.backgroundImage = "assets/videoplayer_audio_slider.png";
	audioSliderButtonStyle.activeBackgroundImage = "assets/videoplayer_audio_slider.png";
//	audioSliderButtonStyle.borderColor = RAYLIB_GRAY;
//	audioSliderButtonStyle.borderWidth = 2;
	audioSliderButtonStyle.bounds = {664, 31, 122, 51};
	styleManager.addClassStyle("audio_slider", audioSliderButtonStyle);
}