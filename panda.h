/**
 * \file panda.h
 * \author Laurens Wuyts
 * \date 10 May 2018
 * \brief File containing all panda specific function declarations.
 *
 * This file contains all the function declarations for using the panda, as well as the definition of the Panda struct. 
 */

#ifndef PANDA
#define PANDA
	#include <libusb-1.0/libusb.h>

        /**
	 * \brief Defines the interface for a specific connected Panda.
	 * 
	 * This struct contains the USB handle and file descriptor, so it can be passed to all functions.
	 */
	typedef struct {
	    libusb_device_handle *handle;		//!< The LibUSB handle
	    struct libusb_device_descriptor desc;	//!< The LibUSB file descriptor
	} Panda;

	/**
	 * \brief Defines a standard CAN frame
	 * 
	 * This struct defines a standard CAN frame, so that the software can be used with different CAN devices with different drivers.
	 * 
	 */
	typedef struct {
	    uint16_t ID;	//!< The CAN frame ID.
	    uint8_t data[8];	//!< The Data sent with the frame, max. 8 Bytes.
	    uint8_t bus;	//!< Which bus to send the data on. For using multiple CAN busses.
	    uint8_t length;	//!< The number of bytes te be sent.
	    uint8_t freq;	//!< How frequent to send the frame. 
	} CANFrame;

        /**
         * \brief Contains a few health parameters of the car and the Panda.
         *
         * This struct contains a few health parameters of the car and the Panda.
         *
         */
        typedef struct {
            uint32_t voltage;                   //!< The car power voltage
            uint32_t current;                   //!< The current drawn by the Panda
            uint8_t started;                    //!< Is the car started?
            uint8_t controls_allowed;           //!< Is it allowed to control the car?
            uint8_t gas_interceptor_detected;   //!<
            uint8_t started_signal_detected;    //!< (Deprecated) Not used anymore
            uint8_t started_alt;                //!< (Deprecated) Not used anymore
        } Health;


	/**
	 * \brief Constant to define what type of request you want to make.
	 * 
	 * These defines are constants to define what type of USB request is made.
	 */
	#define REQUEST_IN (LIBUSB_ENDPOINT_IN  | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE)

	/**
	 * \brief Constant to define what type of request you want to make.
	 * 
	 * These defines are constants to define what type of USB request is made.
	 */
	#define REQUEST_OUT (LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE)

	/**
         * \fn int panda_setup(Panda *p, int mode)
	 * \brief Setup and connect to the Panda
	 * \param p Pointer to Panda struct.
         * \param mode Safety Mode
         * \return 0: Success
         * \return <0: Fail
	 * 
	 * \fn int panda_connect(Panda *p)
	 * \brief Connect to the Panda (Called from setup)
	 * \param p Pointer to Panda struct.
         * \return 0: Success
         * \return <0: Fail
	 * 
	 * \fn int panda_close(Panda *p)
	 * \brief Close the USB handle of the Panda
	 * \param p Pointer to Panda struct.
         * \return 0: Success
         * \return <0: Fail
	 * 
	 * \fn int panda_get_version(Panda *p)
	 * \brief Retrieve and print the current version of the Panda firmware.
	 * \param p Pointer to Panda struct.
         * \return 0: Success
         * \return <0: Fail
	 * 
	 * \fn int panda_set_safety_mode(Panda *p, uint16_t mode)
	 * \brief Set the safety mode of the Panda, to allow sending on the CAN busses
	 * \param p Pointer to Panda struct.
	 * \param mode Mode to set the Panda to. (0 = listen only, 0x1337 = Write all)
         * \return 0: Success
         * \return <0: Fail
	 * 
	 * \fn int panda_set_can_speed(Panda *p, int bus, int speed)
	 * \brief Set the speed of a specific CAN bus of the Panda
	 * \param p Pointer to Panda struct.
	 * \param bus Which bus to change
	 * \param speed The speed to set in kbps
         * \return 0: Success
         * \return <0: Fail
	 * 
         * \fn int panda_get_health(Panda *p, Health *h)
         * \brief Get the car health from the Panda
         * \param p Pointer to Panda struct
         * \param h Struct to store the read data.
         * \return 0: Success
         * \return <0: Fail
         *
	 * \fn int panda_can_send_many(Panda *p, CANFrame frames[], int length)
	 * \brief Send many CAN frames to the Panda
	 * \param p Pointer to Panda struct.
	 * \param frames The CAN frames to send to the Panda.
	 * \param length The number of CAN frames to send.
         * \return 0: Success
         * \return <0: Fail
	 * 
	 * \fn int panda_can_send(Panda *p, CANFrame frame)
	 * \brief Send one CAN frame to the Panda
	 * \param p Pointer to Panda struct.
	 * \param frame The frame to send.
         * \return 0: Success
         * \return <0: Fail
	 * 
	 * \fn int panda_can_recv(Panda *p, unsigned char *data, int length)
	 * \brief Request received CAN frames from the Panda
	 * \param p Pointer to Panda struct.
	 * \param data The received data from the Panda.
	 * \param length The maximum quantity of data to request.
         * \return 0: Success
         * \return <0: Fail
	 * 
	 * \fn int panda_can_clear(Panda *p, int bus)
	 * \brief Clear an internal buffer of the Panda
	 * \param p Pointer to Panda struct.
	 * \param bus The bus to clear the buffer of.
         * \return 0: Success
         * \return <0: Fail
	 * 
	 * \fn void print_many(CANFrame frames[], int length)
	 * \brief Debug the frames that would be sent.
	 * \param frames The frames to print.
	 * \param length The number of frames to print.
         * \return 0: Success
         * \return <0: Problem
	 */
        int panda_setup(Panda *p, int mode);
	int panda_connect(Panda *p);
	int panda_close(Panda *p);

	int panda_get_version(Panda *p);
	int panda_set_safety_mode(Panda *p, uint16_t mode);
	int panda_set_can_speed(Panda *p, int bus, int speed);
        int panda_get_health(Panda *p, Health *h);

	int panda_can_send_many(Panda *p, CANFrame frames[], int length);
	int panda_can_send(Panda *p, CANFrame frame);
	int panda_can_recv(Panda *p, unsigned char *data, int length);
	int panda_can_clear(Panda *p, int bus);

	void print_many(CANFrame frames[], int length);
#endif	
