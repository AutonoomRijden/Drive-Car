/**
 * \file joystick.h
 * \author Laurens Wuyts
 * \date 28 May 2018
 * \brief File containing the library to use a gamepad in Linux.
 *
 * This file contains all the function declarations for using the panda, as well as the definition of the Panda struct.
 */

#ifndef JOYSTICK
#define JOYSTICK

    /**
     * \brief Contains the X and Y value of an axis
     */
    typedef struct {
        int16_t x;  //!< The X value of a joystick axis
        int16_t y;  //!< The Y value of a joystick axis
    } Axis;


    /**
     * \brief Defines the interface for a connected joystick/gamepad.
     *
     * This struct contains the definition of a joystick or gamepad so it can be passed between functions.
     */
    typedef struct {
        const char *name;       //!< The Name of the interface.
        int fd;                 //!< The file desctriptor to read from and write to.
        Axis axes[3];           //!< The values of all axes of the gamepad.
        uint8_t buttons[12];    //!< The state of all buttons on the joystick/gamepad
        uint8_t numberOfAxes;   //!< The number of axes that the specific joystick has.
        uint8_t numberOfButtons;//!< The number of buttons that a specific joystick has.
    } Joystick;

    /**
     * \fn int setupJoystick(Joystick *js, char *name)
     * \brief Setup and connect to a joystick
     * \param js Pointer to Joystick struct.
     * \param name The name of the joystick to connect to.
     * \return 0: Success
     * \return <0: Fail
     *
     * \fn int readJoystick(Joystick *js)
     * \brief Reads te current status of the joystick and puts it in the struct
     * \param js Pointer to Joystick struct.
     * \return 0: Success
     * \return <0: Fail
     *
     * \fn void printState(Joystick *js, int enableAxes, int enableButtons)
     * \brief Debugs the status of the joystick. Can be configured to show only the axes, the buttons or both.
     * \param js Pointer to Joystick struct.
     * \param enableAxes Prints the values of the axes.
     * \param enableButtons Prints the values of all the buttons.
     */

    int setupJoystick(Joystick *js, char *name);
    int readJoystick(Joystick *js);
    void printState(Joystick *js, int enableAxes, int enableButtons);
#endif
