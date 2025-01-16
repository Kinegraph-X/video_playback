#pragma once

struct MediaState {
    enum State {UNDEFINED, LOADED, BUFFERING, ENOUGHDATATOPLAY, PLAYING, PAUSED, ENDED};
	State status = State::UNDEFINED;
    
};