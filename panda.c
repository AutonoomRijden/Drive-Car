#include <stdio.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

#include "panda.h"

#define terminalColor(color) printf("\033[%dm", color)

int panda_setup(Panda *p) {
    p->handle = 0;
    int ret;

    ret = libusb_init(NULL);

    if(ret < 0) {
        terminalColor(31);
        printf("Unable to Init\n");
        terminalColor(0);
        return ret;
    }

    ret = panda_connect(p);

    if(ret < 0) {
        terminalColor(31);
        printf("Unable to connect\n");
        terminalColor(0);
        return ret;
    }

    panda_set_safety_mode(p, 0x1337);

    return 0;
}

int panda_connect(Panda *p) {
    libusb_device **devices;
    ssize_t cnt;
    int ret;

    if(p->handle != 0)
        panda_close(p);

    cnt = libusb_get_device_list(NULL, &devices);
    if(cnt < 0) {
        terminalColor(31);
        printf("No devices\n");
        terminalColor(0);
        return (int)cnt;
    }

    for(int i = 0; devices[i]; ++i) {
        ret = libusb_get_device_descriptor(devices[i], &(p->desc));
        if(ret < 0) {
            terminalColor(31);
            printf("Failed to get descriptor\n");
            terminalColor(0);
            return ret;
        }

        //printf("%d %04x %04x\n", i, p->desc.idVendor, p->desc.idProduct);
        if(p->desc.idVendor == 0xbbaa && (p->desc.idProduct == 0xddcc || p->desc.idProduct == 0xddee)) {
            ret = libusb_open(devices[i], &(p->handle));
            if(ret < 0) {
                terminalColor(31);
                printf("Couldn't open device\n");
                terminalColor(0);
                return ret;
            }

            ret = libusb_set_configuration(p->handle, 1);
            if(ret < 0) {
                terminalColor(31);
                printf("%d: Couldn't set configuration\n", ret);
                terminalColor(0);
                return ret;
            }

            ret = libusb_claim_interface(p->handle, 0);
            if(ret < 0) {
                terminalColor(31);
                printf("%d: Couldn't claim interface\n", ret);
                terminalColor(0);
                return ret;
            }

            libusb_control_transfer(p->handle, 0xc0, 0xd9, 0, 0, NULL, 0, 0);

            break;
        }
    }

    if(p->handle == 0) {
        terminalColor(31);
        printf("No Panda found.\n");
        terminalColor(0);
        return -1;
    }
    terminalColor(32);
    printf("Panda connected\n");
    terminalColor(0);
    fflush(stdout);

    return 0;
}

int panda_close(Panda *p) {
    libusb_close(p->handle);
    p->handle = 0;
    terminalColor(32);
    printf("Closed Panda\n");
    terminalColor(0);

    return 0;
}

int panda_get_version(Panda *p) {
    unsigned char data[0x40] = {0};
    int ret;
    ret = libusb_control_transfer(p->handle, REQUEST_IN, 0xd6, 0, 0, data, 0x40, 0);
    if(ret < 0) {
        return ret;
    }

    printf("%s\n", data);

    return 0;
}

int panda_set_safety_mode(Panda *p, uint16_t mode) {
    return libusb_control_transfer(p->handle, REQUEST_OUT, 0xdc, mode, 0, NULL, 0, 0);
}

int panda_set_can_speed(Panda *p, int bus, int speed) {
    unsigned char data[1];
    return libusb_control_transfer(p->handle, REQUEST_OUT, 0xde, bus, speed*10, data, 0, 0);
}

int panda_can_send_many(Panda *p, CANFrame frames[], int length) {
    int nrBytes = 16  * length;
    unsigned char *data = calloc(nrBytes, sizeof(unsigned char));
    unsigned char *tempData = data;
    int transferred;
    int ret;

    for(int i = 0; i < length; i++) {
        tempData[0] = (frames[i].ID >> 3) & 0xFF;
        tempData[1] = (frames[i].ID << 5) & 0xFF;
        tempData[3] = 1;
        tempData[7] = frames[i].length | (frames[i].bus << 4);

        tempData += 8;
        for(int j = 0; j < frames[i].length; j++) {
            tempData[j] = frames[i].data[j];
        }

        tempData += 8;
    }

    ret = libusb_bulk_transfer(p->handle, 3 | LIBUSB_ENDPOINT_OUT, data, nrBytes, &transferred, 0);
    free(data);

    if(ret < 0) {
        return ret;
    }

    return 0;
}

int panda_can_send(Panda *p, CANFrame frame) {
    return panda_can_send_many(p, &frame, 1);
}

int panda_can_recv(Panda *p, unsigned char *data, int length) {
    int transferred;
    int ret;
    ret = libusb_bulk_transfer(p->handle, 1 | LIBUSB_ENDPOINT_IN, data, length, &transferred, 0);
    if(ret < 0) {
        return ret;
    }

    return transferred;
}

int panda_can_clear(Panda *p, int bus) {
    unsigned char data[1];
    return libusb_control_transfer(p->handle, REQUEST_OUT, 0xf1, bus, 0, data, 0, 0);
}

void print_many(CANFrame frames[], int length) {
    for(int k = 0; k < length; k++) {
        printf("Bus: %d  ID: %4d  Length: %d  Data: ", frames[k].bus, frames[k].ID, frames[k].length);
        for(int l = 0; l < frames[k].length; l++) {
            printf("%02X ", frames[k].data[l]);
        }
        printf("\n");
    }
}
