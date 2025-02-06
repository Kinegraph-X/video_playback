#pragma once
#include "player_headers.h"
#include "RaylibManager.h"
#include "DOM/RootNode.h"
#include "DOM/Button.h"
#include "DOM/EventQueue.h"
#include "DOM/UIEvent.h"

void prepareInteractions(RaylibManager& manager, Node* rootNode, std::shared_ptr<EventQueue> eventQueue, AVCodecContext* codecContext) {
	std::vector<Node*> buttons;
	Node* shutdownButton = rootNode->children.at(1)->children.at(7);
	Clickable* handButton = dynamic_cast<Clickable*>(rootNode->children.at(1)->children.at(6));
	
	// Handle window closing for the main loop to exit gracefully 
	shutdownButton->addEventListener(EventType::MouseUp, [rootNode, eventQueue](EventPayload payload) {
		logger(LogLevel::DEBUG, "Closing window on click...");
		EventPayload forwardedPayload{EventType::Close};
        UIEvent event(EventType::Close, rootNode);
		event.payload = forwardedPayload;
        eventQueue->pushEvent(event);;
	});
	
	handButton->onDragMove = [&manager, codecContext](const RaylibVector2 offset) {
		logger(LogLevel::DEBUG, "Move window by offset X : " + std::to_string(offset.x));
		logger(LogLevel::DEBUG, "Move window by offset Y : " + std::to_string(offset.y));
		RaylibVector2 sizeOffset{
            offset.x,  
            offset.y
        };
        manager.moveWindow(sizeOffset);
//		 if (codecContext) {  
//	        WindowSize windowSize = WindowSize{
//	            manager.windowWidth + static_cast<int>(std::round(offset.x)), 
//	            manager.windowHeight + static_cast<int>(std::round(offset.y)), 
//	            0,
//	            0
//	        };
//	        manager.resizeWindowFileLoaded(codecContext, windowSize);  // ✅ Works now
//	    }
//	    else {
//	        WindowSizeOffset sizeOffset{
//	            static_cast<int>(std::round(offset.x)),  
//	            static_cast<int>(std::round(offset.y))
//	        };
//	        manager.resizeWindow(sizeOffset);
//	    }
	};
}