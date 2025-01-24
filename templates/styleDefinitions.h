#include "../StyleManager.h"

// Create and populate the StyleManager
StyleManager styleManager;

// Define styles for IDs
Style mainContainerStyle;
mainContainerStyle.backgroundColor = RAYWHITE;
mainContainerStyle.bounds = {0, 0, 800, 600};
styleManager.addIdStyle("main_container", mainContainerStyle);

// Define styles for classes
Style buttonStyle;
buttonStyle.backgroundColor = LIGHTGRAY;
buttonStyle.textColor = BLACK;
buttonStyle.bounds = {0, 0, 100, 50};
buttonStyle.fontSize = 20;
styleManager.addClassStyle("button", buttonStyle);

Style playButtonStyle = buttonStyle;
playButtonStyle.backgroundColor = GREEN;
styleManager.addClassStyle("play_button", playButtonStyle);

Style stopButtonStyle = buttonStyle;
stopButtonStyle.backgroundColor = RED;
styleManager.addClassStyle("stop_button", stopButtonStyle);