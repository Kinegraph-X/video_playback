#include "Style.h"

const PropertyValue<Position> DefaultStyle::position{"position", Position::Relative};
const PropertyValue<RaylibRectangle> DefaultStyle::bounds{"bounds", {0, 0, 0, 0}};
const PropertyValue<int> DefaultStyle::zIndex{"zIndex", 0};
const PropertyValue<RaylibColor> DefaultStyle::backgroundColor{"backgroundColor", RAYLIB_BLANK};
const PropertyValue<std::string> DefaultStyle::backgroundImage{"backgroundImage", ""};
const PropertyValue<RaylibColor> DefaultStyle::borderColor{"borderColor", RAYLIB_BLACK};
const PropertyValue<RaylibColor> DefaultStyle::textColor{"textColor", RAYLIB_BLACK};
const PropertyValue<float> DefaultStyle::borderWidth{"borderWidth", 0.0f};
const PropertyValue<int> DefaultStyle::borderRadius{"borderRadius", 0};
const PropertyValue<int> DefaultStyle::fontSize{"fontSize", 16};
const PropertyValue<std::string> DefaultStyle::fontName{"fontName", "default"};
const PropertyValue<bool> DefaultStyle::isVisible{"isVisible", true};
const PropertyValue<std::string> DefaultStyle::error{"error", "static error value"};