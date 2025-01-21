#pragma once

struct MediaState {
    enum State {UNDEFINED, LOADED, BUFFERING, ENOUGHDATATOPLAY, PLAYING, PAUSED, ENDING, ENDED};
	State status = State::UNDEFINED;
    
};