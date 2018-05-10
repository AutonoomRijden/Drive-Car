#ifndef JOYSTICK
#define JOYSTICK
	typedef struct {
	    int16_t x;
	    int16_t y;
	} axisState;

	typedef struct {
	    const char *name;
	    int fd;
	    axisState axes[3];
	    uint8_t buttons[12];
	    uint8_t numberOfAxes;
	    uint8_t numberOfButtons;
    } Joystick;

    int setupJoystick(Joystick *js, char *name);
    int readJoystick(Joystick *js);
    void printState(Joystick *js, int enableAxes, int enableButtons);
#endif
