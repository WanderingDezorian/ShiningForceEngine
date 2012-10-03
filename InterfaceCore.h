#ifndef INTERFACECORE_H
#define INTERFACECORE_H

#include <ctime>

class InterfaceCore{
	private:
		char KeyState;
		char KeyPressed;
	public:
		enum KeyMask{
			KEY_UP = 0x01,
			KEY_DOWN = 0x02,
			KEY_LEFT = 0x04,
			KEY_RIGHT = 0x08,
			KEY_OK = 0x10,
			KEY_CANCEL = 0x20,
			KEY_START = 0x40,
			KEY_EXIT = 0x80
		};

		InterfaceCore() : KeyState(0), KeyPressed(0) {};
		~InterfaceCore(){}

		void PollInterface(clock_t AbortTime); // Do one step-through, normally called by graphics mode
		char GetButtonState(){
			char RetVal = KeyPressed;
			KeyPressed = KeyState;
			return RetVal;
		}

		char PeekButtonState(){ return KeyState; }
		char PeekButtonPressed(){ return KeyPressed; }

		// Maybe?  void MainInterfaceLoop();
};

#endif // INTERFACECORE_H
