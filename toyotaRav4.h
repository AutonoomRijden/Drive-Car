/**
 * \file toyotaRav4.h
 * \author Laurens Wuyts
 * \date 28 May 2018
 * \brief File containing all Toyota Rav4 specific function declarations.
 *
 * This file contains all the function declarations for send all the right CAN bus messages for controlling a Toyota Rav4 Hybrid.
 */

#ifndef TOYOTA_RAV4
#define TOYOTA_RAV4
    #include <stdint.h>
    #include "panda.h"

    #define ARRAY_LENGTH(arr)  (sizeof(arr) / sizeof((arr)[0]))
    /**
     * \fn uint16_t create_checksum(CANFrame *frame)
     * \brief Calculate the checksum of the CAN frame.
     * \param frame The frame to calculate the checksum for.
     * \return The calculated checksum.
     *
     * \fn int sendStaticVideo(CANFrame frames[], uint16_t count)
     * \brief Send the static messages to replace the video from the camera.
     * \param frames The array to add the messages to.
     * \param count The 100Hz counter of the program.
     * \return Number of messages added.
     *
     * \fn int sendStaticCam(CANFrame frames[], uint16_t count)
     * \brief Send the static messages to replace the camera.
     * \param frames The array to add the messages to.
     * \param count The 100Hz counter of the program.
     * \return Number of messages added.
     *
     * \fn int sendStaticDsu(CANFrame frames[], uint16_t count)
     * \brief Send the static messages to replace the DSU.
     * \param frames The array to add the messages to.
     * \param count The 100Hz counter of the program.
     * \return Number of messages added.
     *
     * \fn int sendSteerCommand(CANFrame frames[], uint16_t count, uint16_t torque)
     * \brief Send the message to control the steering wheel.
     * \param frames The array to add the messages to.
     * \param count The 100Hz counter of the program.
     * \param torque The amount of torque to add to the steering wheel.
     * \return Number of messages added.
     *
     * \fn int sendAccelCommand(CANFrame frames[], uint16_t count, uint16_t acceleration, uint8_t cancel)
     * \brief Send the message to control the acceleration and braking of the car.
     * \param frames The array to add the messages to.
     * \param count The 100Hz counter of the program.
     * \param acceleration The force to accelerate or decelerate with. (Negative is decelerate)
     * \param cancel Bit to cancel the controls and turn of cruise control.
     * \return Number of messages added.
     *
     * \fn int sendUiCommand(CANFrame frames[], uint16_t count, uint8_t status)
     * \brief Send the messages to control the heads up display.
     * \param frames The array to add the messages to.
     * \param count The 100Hz counter of the program.
     * \param status The status of the heads up display.
     * \return Number of messages added.
     *
     * \fn int sendFcwCommand(CANFrame frames[], uint16_t count, uint8_t fcw)
     * \brief Send the message to enable or disable Forward Collision Warning.
     * \param frames The array to add the messages to.
     * \param count The 100Hz counter of the program.
     * \param fcw Enable/Disable the Forward Collision Warning.
     * \return Number of messages added.
     */

    uint16_t create_checksum(CANFrame *frame);
    int sendStaticVideo(CANFrame frames[], uint16_t count);
    int sendStaticCam(CANFrame frames[], uint16_t count);
    int sendStaticDsu(CANFrame frames[], uint16_t count);
    int sendSteerCommand(CANFrame frames[], uint16_t count, uint16_t torque);
    int sendAccelCommand(CANFrame frames[], uint16_t count, uint16_t acceleration, uint8_t cancel);
    int sendUiCommand(CANFrame frames[], uint16_t count, uint8_t status);
    int sendFcwCommand(CANFrame frames[], uint16_t count, uint8_t fcw);
#endif
