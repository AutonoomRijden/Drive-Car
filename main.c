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

#define ARRAY_LENGTH(arr)  (sizeof(arr) / sizeof((arr)[0]))

typedef struct {
    char *js;
    uint8_t enableDsu;
    uint8_t enableCam;
} Params;

typedef struct timeval Time;

typedef struct {
    float speed;
    float angle;
} CARState;

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

uint16_t create_checksum(CANFrame *frame) {
    uint16_t checksum = (frame->ID >> 8) + (frame->ID & 0xFF) + frame->length;

    for(uint8_t i = 0; i < frame->length - 1; i++)
        checksum += frame->data[i];

    frame->data[frame->length - 1] = checksum;
    return checksum;
}
int sendStaticVideo(CANFrame frames[], uint16_t count) {
    const uint16_t addr_vid[] = {
        0x340, 0x341, 0x342, 0x343, 0x344, 0x345,
        0x363, 0x364, 0x365, 0x370, 0x371, 0x372,
        0x373, 0x374, 0x375, 0x380, 0x381, 0x382,
        0x383
    };

    const uint8_t length_vid = ARRAY_LENGTH(addr_vid);

    CANFrame static_vid = {0x000, {0x00, 0x03, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, 8, 10};
    uint16_t cks;
    uint8_t add_length = 0;

    if(count % 10 == 0) {
        static_vid.data[0] = (count / 10) & 0xFF;
        cks = create_checksum(&static_vid);
        for(uint8_t i = 0 ; i < length_vid; i++) {
            frames[i] = static_vid;
            frames[i].data[7] = cks + (addr_vid[i] >> 8) + (addr_vid[i] & 0xFF);
            frames[i].ID = addr_vid[i];
            add_length++;
        }
    }

    return add_length;
}
int sendStaticCam(CANFrame frames[], uint16_t count) {
    CANFrame static_cam[] = {
        {0x367, {0x06, 0x00},                                       0, 2, 40},
        {0x414, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00},   0, 8, 100},
        {0x489, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   0, 8, 100},
        {0x48A, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   0, 8, 100},
        {0x48B, {0x66, 0x06, 0x08, 0x0A, 0x02, 0x00, 0x00, 0x00},   0, 8, 100},
        {0x4D3, {0x1C, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00},   0, 8, 100},
        {0x130, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38},         1, 7, 100},
        {0x240, {0x00, 0x00, 0x10, 0x01, 0x00, 0x10, 0x01, 0x00},   1, 8, 5},
        {0x241, {0x00, 0x00, 0x10, 0x01, 0x00, 0x10, 0x01, 0x00},   1, 8, 5},
        {0x244, {0x00, 0x00, 0x10, 0x01, 0x00, 0x10, 0x01, 0x00},   1, 8, 5},
        {0x245, {0x00, 0x00, 0x10, 0x01, 0x00, 0x10, 0x01, 0x00},   1, 8, 5},
        {0x248, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},   1, 8, 5},
        {0x466, {0x20, 0x20, 0xAD},                                 1, 3, 100}
    };

    const uint8_t length_cam = ARRAY_LENGTH(static_cam);
    uint8_t add_length = 0;

    for(uint8_t i = 0; i < length_cam; i++) {
        if(count % static_cam[i].freq == 0) {
            if(static_cam[i].freq == 5)
                static_cam[i].data[0] = (((count / 5) % 7) + 1) << 5;
            else if(static_cam[i].ID == 0x489 || static_cam[i].ID == 0x48A)
                static_cam[i].data[7] = ((static_cam[i].ID & 0x002) << 6) + ((count / 100) % 0xF) + 1;

            frames[i] = static_cam[i];
            add_length++;
        }
    }

    return add_length;
}
int sendStaticDsu(CANFrame frames[], uint16_t count) {
    CANFrame static_dsu[] = {
        {0x141, {0x00, 0x00, 0x00, 0x46},                           1, 4, 2},
        {0x128, {0xF4, 0x01, 0x90, 0x83, 0x00, 0x37},               1, 6, 3},
        {0x283, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8C},         0, 7, 3},
        {0x2E6, {0xFF, 0xF8, 0x00, 0x08, 0x7F, 0xE0, 0x00, 0x4E},   0, 8, 3},
        {0x2E7, {0xA8, 0x9C, 0x31, 0x9C, 0x00, 0x00, 0x00, 0x02},   0, 8, 3},
        {0x344, {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50},   0, 8, 5},
        {0x160, {0x00, 0x00, 0x08, 0x12, 0x01, 0x31, 0x9C, 0x51},   1, 8, 7},
        {0x161, {0x00, 0x1E, 0x00, 0x00, 0x00, 0x80, 0x07},         1, 7, 7},
        {0x33E, {0x0F, 0xFF, 0x26, 0x40, 0x00, 0x1F, 0x00},         0, 7, 20},
        {0x365, {0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x08},         0, 7, 20},
        {0x366, {0x00, 0x00, 0x4D, 0x82, 0x40, 0x02, 0x00},         0, 7, 20},
        {0x4CB, {0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   0, 8, 100},
        {0x470, {0x00, 0x00, 0x02, 0x7A},                           1, 4, 100}
    };

    const uint8_t length_dsu = ARRAY_LENGTH(static_dsu);
    uint8_t add_length = 0;

    for(uint8_t i = 0; i < length_dsu; i++) {
        if(count % static_dsu[i].freq == 0) {
            frames[i] = static_dsu[i];
            add_length++;
        }
    }

    return add_length;
}
int sendSteerCommand(CANFrame frames[], uint16_t count, uint16_t torque) {
    uint8_t cnt = ((count & 0x3F) << 1) | 0x80;
    if(torque)
        cnt |= 1;

    /** *************************************
     * Hud:                                 *
     * 0x00 - Regular                       *
     * 0x40 - Actively Steering (beep)      *
     * 0x80 - Actively Steering (no beep)   *
     ************************************* **/
    CANFrame frame;
    frame.ID = 0x2E4;
    frame.length = 5;
    frame.data[0] = cnt;
    frame.data[1] = torque >> 8;
    frame.data[2] = torque & 0xFF;
    frame.data[3] = 0x00;  // Hud
    create_checksum(&frame);
    frames[0] = frame;
    frames[0].bus = 0;

    return 1;
}
int sendAccelCommand(CANFrame frames[], uint16_t count, uint16_t acceleration) {
    if(count % 3 == 0) {
        frames[0].ID = 0x343;
        frames[0].length = 8;
        frames[0].data[0] = acceleration >> 8;
        frames[0].data[1] = acceleration & 0xFF;
        frames[0].data[2] = 0x63;
        frames[0].data[3] = 0xC0;
        frames[0].data[4] = 0x00;
        frames[0].data[5] = 0x00;
        frames[0].data[6] = 0x00;
        create_checksum(&frames[0]);
        frames[0].bus = 0;
        return 1;
    }

    return 0;
}
int sendUiCommand(CANFrame frames[], uint16_t count, uint8_t status) {
    uint8_t sound_1 = status & 0x01;
    uint8_t sound_2 = (status & 0x02) << 3;
    uint8_t steer   = (status & 0x04) >> 2;

    if(count % 100 == 0) {
        frames[0].ID = 0x412;
        frames[0].length = 8;
        frames[0].data[0] = 0x54;
        frames[0].data[1] = 0x04 + steer + sound_2;
        frames[0].data[2] = 0x0C;
        frames[0].data[3] = 0x00;
        frames[0].data[4] = sound_1;
        frames[0].data[5] = 0x2C;
        frames[0].data[6] = 0x38;
        frames[0].data[7] = 0x02;
        frames[0].bus = 0;

        return 1;
    }

    return 0;
}
int sendFcwCommand(CANFrame frames[], uint16_t count, uint8_t fcw) {
    if(count % 100 == 0) {
        frames[0].ID = 0x411;
        frames[0].length = 8;
        frames[0].data[0] = fcw << 4;
        frames[0].data[1] = 0x20;
        frames[0].data[2] = 0x00;
        frames[0].data[3] = 0x00;
        frames[0].data[4] = 0x10;
        frames[0].data[5] = 0x00;
        frames[0].data[6] = 0x80;
        frames[0].data[7] = 0x00;
        frames[0].bus = 0;

        return 1;
    }

    return 0;
}
void analyzeCANFrame(CANFrame frame, CARState *state) {
    int16_t temp;
    switch(frame.ID) {
        case 0x024:     // Angle
            temp = (frame.data[0] << 8) + frame.data[1];
            temp = (temp >> 11) ? (0xF000 | temp) : temp;  // 12 bit 2s complement to 16 bit
            state->angle = temp * 1.5;
            break;
        case 0x0B4:     // Speed
            state->speed = ((frame.data[5] << 8) + frame.data[6]) / 100.0;
            break;
        default:
            break;
    }
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
    //CANFrame frame;
    CANFrame frame_list[256];
    uint8_t list_length = 0;

    Time time;
    Time prev_time;

    //CARState state;

    Params params;

    ret = getParams(argc, argv, &params);
    if(ret < 0) goto end;
    ret = panda_setup(&p);
    if(ret < 0) goto end;
    ret = setupJoystick(&js, params.js);
    if(ret < 0) goto end;
    gettimeofday(&prev_time, NULL);

    while(running) {
        gettimeofday(&time, NULL);

        //readCanFrame(ci[0], &frame);
        //analyzeCANFrame(frame, &state);

        readJoystick(&js);
        // 100 Hz
        if((time.tv_usec + ((time.tv_sec - prev_time.tv_sec) * 1000000)) >= (prev_time.tv_usec + 10000)) {
            //printState(&js, 1, 0);

            if(params.enableCam) {
                //list_length += sendSteerCommand(frame_list, count, js.axes[0].x/10);           // Cam
                list_length += sendSteerCommand(frame_list, count, 0);           // Cam

                list_length += sendStaticVideo(frame_list + list_length, count);               // Cam
                list_length += sendStaticCam(frame_list + list_length, count);                 // Cam

                list_length += sendUiCommand(frame_list + list_length, count, 0);              // Cam
                list_length += sendFcwCommand(frame_list + list_length, count, 0);             // Cam
            }

            if(params.enableDsu) {
                //list_length += sendAccelCommand(frame_list + list_length, count, (js.axes[0].y * -1)/10);    // Dsu
                list_length += sendAccelCommand(frame_list + list_length, count, 0);    // Dsu
                list_length += sendStaticDsu(frame_list + list_length, count);                               // Dsu
            }

            count++;
            prev_time.tv_usec = time.tv_usec;
            prev_time.tv_sec  = time.tv_sec;

            if(list_length > 0) {
                //print_many(frame_list, list_length);
                panda_can_send_many(&p, frame_list, list_length);
            }

            list_length = 0;
        }

        //if(count == 1) break;

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
