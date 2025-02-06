#pragma once

enum class EventType {
	None,
	Close,
    MouseDown,
    MouseUp,
    MouseMove,
    MouseEnter,
    MouseOut,
    KeyDown,
    KeyUp,
    CharInput,
    DragStart,
    DragMove,
    DragEnd,
    MouseWheel,
    // Add more event types as needed
};
