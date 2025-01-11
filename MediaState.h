#pragma once

struct MediaState {
    enum State {UNDEFINED, LOADED, PLAYING, PAUSED, ENDED};
	State status = State::UNDEFINED;
    
};