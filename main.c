/*! \mainpage Drive Car
 *
 * \section intro_sec Introduction
 *
 * This project aims to drive a car from a Linux pc using a game controller.\n
 * For communication with the car interfaces, a comma.ai Panda is used.\n
 * The comma.ai Panda is talked to via USB and the libusb. \n
 *
 * \section build_sec Build
 *
 * To build this project, you can just run\n
 * ```make```\n\n
 * To clean all the build files and the compiled software, run\n
 * ```make clean```\n
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/time.h>

#include "panda.h"
#include "joystick.h"
#include "toyotaRav4.h"

typedef struct {
    char *js;
    uint8_t enableDsu;
    uint8_t enableCam;
} Params;

typedef struct timeval Time;

#define terminalColor(color) printf("\033[%dm", color)


int getParams(int argc, char *argv[], Params *params) {
    if(argc <= 1) {
        printf("%s \033[31m<cam-dsu>\033[32m [<js>]\033[0m\n"
               " cam-dsu\t C, D or CD\n"
               " js\t\t Joystick/Gamepad\t(default: /dev/input/js0)\n", argv[0]);

        return -1;
    }

    params->js = (argc > 4) ? argv[4] : "/dev/input/js0";

    if(argv[1][0] == 'C')
        params->enableCam = 1;
    if(argv[1][0] == 'D' || argv[1][1] == 'D')
        params->enableDsu = 1;
    if(!(params->enableCam || params->enableDsu))
        getParams(0, argv, NULL);

    return 0;
}

uint8_t running = 1;
void signal_handler(int signal) {
    running = 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);

    int ret;
    uint16_t count = 0;

    Joystick js;
    Panda p;
    CANFrame frame_list[256];
    uint8_t list_length = 0;

    Time time;
    Time prev_time;

    Params params;

    Health h;

    ret = getParams(argc, argv, &params);
    if(ret < 0) goto end;
    ret = panda_setup(&p, 0x1336);
    if(ret < 0) goto end;
    ret = setupJoystick(&js, params.js);
    if(ret < 0) goto end;
    gettimeofday(&prev_time, NULL);

    panda_get_health(&p, &h);
    printf("V:%d  Started:%d  Controls:%d\n", h.voltage, h.started, h.controls_allowed);

    int16_t steer;
    int16_t steer_count;
    int16_t accel = 0;
    int16_t decel = 0;

    while(running) {
        gettimeofday(&time, NULL);

        readJoystick(&js);

        // 100 Hz
        if((time.tv_usec + ((time.tv_sec - prev_time.tv_sec) * 1000000)) >= (prev_time.tv_usec + 10000)) {
            if(params.enableCam) {
                steer = (js.axes[0].x * (-1))/22;
                //steer = (steer > 1500) ? 1500 : ((steer < -1500) ? -1500 : steer);
                if(steer > (steer_count + 30))
                    steer_count += 30;
                if(steer < (steer_count - 30))
                    steer_count -= 30;

                if(steer == 0)
                    steer_count = 0;

                list_length += sendSteerCommand(frame_list, count, steer_count);               // Cam

                list_length += sendStaticVideo(frame_list + list_length, count);               // Cam
                list_length += sendStaticCam(frame_list + list_length, count);                 // Cam

                list_length += sendUiCommand(frame_list + list_length, count, 0);              // Cam
                list_length += sendFcwCommand(frame_list + list_length, count, 0);             // Cam
            }

            if(params.enableDsu) {
                accel = (js.buttons[1] * !js.buttons[2] * (accel + 10));
                decel = (js.buttons[2] * (decel - 20));

                accel = (accel > 1500) ? 1500 : accel;
                decel = (decel < -3000) ? -3000 : decel;
                list_length += sendAccelCommand(frame_list + list_length, count, accel + decel, js.buttons[3]);  // Dsu
                list_length += sendStaticDsu(frame_list + list_length, count);                               // Dsu
            }

            count++;
            prev_time.tv_usec = time.tv_usec;
            prev_time.tv_sec  = time.tv_sec;

            if(list_length > 0) {
                panda_can_send_many(&p, frame_list, list_length);
            }

            list_length = 0;
        }

        usleep(10);
    }

    printf("\n");

    end:
    if(js.fd != 0) {
        close(js.fd);
        terminalColor(32);
        printf("Closed Joystick\n");
        terminalColor(0);
    }
    if(p.handle != 0) panda_close(&p);
    return 0;
}
