#include <SDL/SDL.h>
#include "InterfaceCore.h"

void InterfaceCore::PollInterface(clock_t AbortTime){
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_KEYDOWN){
			switch(event.key.keysym.sym){ // TODO:  Use a std::map to map keycodes to masks.  This allows users to configure their keys.
				case SDLK_LCTRL:
					KeyState |= KEY_OK;
					break;
				case SDLK_LALT:
					KeyState |= KEY_CANCEL;
					break;
				case SDLK_UP:
					KeyState |= KEY_UP;
					break;
				case SDLK_DOWN:
					KeyState |= KEY_DOWN;
					break;
				case SDLK_LEFT:
					KeyState |= KEY_LEFT;
					break;
				case SDLK_RIGHT:
					KeyState |= KEY_RIGHT;
					break;
				case SDLK_RETURN:
					KeyState |= KEY_START;
					break;
				case SDLK_ESCAPE:
					KeyState |= KEY_EXIT;
				default:
					break;
			} // end switch
			KeyPressed |= KeyState;
		} // end if(event.type == SDL_KEYDOWN)
		else if(event.type == SDL_KEYUP){
			switch(event.key.keysym.sym){ // TODO:  Use a std::map to map keycodes to masks.  This allows users to configure their keys.
				case SDLK_LCTRL:
					KeyState &= ~KEY_OK;
					break;
				case SDLK_LALT:
					KeyState &= ~KEY_CANCEL;
					break;
				case SDLK_UP:
					KeyState &= ~KEY_UP;
					break;
				case SDLK_DOWN:
					KeyState &= ~KEY_DOWN;
					break;
				case SDLK_LEFT:
					KeyState &= ~KEY_LEFT;
					break;
				case SDLK_RIGHT:
					KeyState &= ~KEY_RIGHT;
					break;
				case SDLK_RETURN:
					KeyState &= ~KEY_START;
				default:
					break;
			} // end switch
		} // end if(event.type == SDL_KEYDOWN)
		else if(event.type == SDL_QUIT){
			KeyState |= KEY_EXIT;
			KeyPressed |= KEY_EXIT;
		}
		if(clock() > AbortTime)
			break;
	} // end while
}
