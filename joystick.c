#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <linux/joystick.h>

#include "joystick.h"

#define terminalColor(color) printf("\033[%dm", color)

int setupJoystick(Joystick *js, char *name) {
    memset(js->axes, 0, 3 * sizeof(axisState));
    memset(js->buttons, 0, 12 * sizeof(uint8_t));

    js->name = name;
    js->fd = open(js->name, O_RDONLY | O_NONBLOCK);
    if(js->fd == -1) {
        terminalColor(31);
        printf("Could not open joystick\n");
        terminalColor(0);
        return -1;
    }

    if(ioctl(js->fd, JSIOCGAXES, &js->numberOfAxes) == -1)
        js->numberOfAxes = 0;

    if(ioctl(js->fd, JSIOCGBUTTONS, &js->numberOfButtons) == -1)
        js->numberOfButtons = 0;

    terminalColor(32);
    printf("Joystick connected\n");
    terminalColor(0);
    fflush(stdout);

    return 0;
}

int readJoystick(Joystick *js) {
    struct js_event event;
    uint8_t axis;

    if(read(js->fd, &event, sizeof(event)) > 0) {
        switch(event.type) {
            case JS_EVENT_BUTTON:
                js->buttons[event.number] = event.value;
                break;
            case JS_EVENT_AXIS:
                axis = event.number / 2;

                if(axis < 3) {
                    if(event.number % 2 == 0)
                        js->axes[axis].x = event.value;
                    else
                        js->axes[axis].y = event.value;
                }
                break;
            default:
                /* Ignore init events. */
                break;
        }
    }

    if(errno != EAGAIN && errno != 0) {
        terminalColor(31);
        printf("%d\n", errno);
        return -1;
    }

    return 0;
}

void printState(Joystick *js, int enableAxes, int enableButtons) {
    if(enableAxes)
        for(uint8_t i = 0; i < js->numberOfAxes / 2; i++) {
            printf("X%d: %6d Y%d: %6d ", i, js->axes[i].x, i, js->axes[i].y);
        }
    if(enableButtons)
        for(uint8_t i = 0; i < js->numberOfButtons; i++) {
            printf("B%d: %d ", i, js->buttons[i]);
        }

    printf("\r");
    fflush(stdout);
}
