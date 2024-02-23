/**
 * @file app.h
 *
 * @author
 *  Ginn Sato
 *
 * @date
 *  01/31/2023
 *
 * @brief
 *  Defines application functions and variables
 *
 */


#ifndef INC_APP_H_
#define INC_APP_H_

#include "stm32f4xx_hal.h"
#include "Gyro_Driver.h"
#include "cmsis_os.h"

#define USER_BUTTON_PORT		GPIOA
#define USER_BUTTON_PIN			GPIO_PIN_0

#define GYRO_DRDY_PORT		    GPIOA
#define GYRO_DRDY_PIN			GPIO_PIN_2

#define LED_RED_PORT			GPIOG
#define LED_GREEN_PORT 			GPIOG

#define LED_RED_PIN				GPIO_PIN_14
#define LED_GREEN_PIN 			GPIO_PIN_13

#define SAMPLE_GYRO_CB		    0b001
#define DRIVE_LED_CB			0b010
#define SAMPLE_BTN_CB			0b100

#define LAB3_USE_TASK			0

// TYPEDEF ENUMS

typedef enum {
	CCW,
	CW
} GYRO_DIRECTION;

typedef enum {
	READY,
	NOT_READY
} Data_Ready_t;


// Function to init everything for our application
void app_init();

// Function to sample the user button
void app_sample_user_button();

// Function to determine the rate of gyro rotation
void app_get_gyro_rate();

// Function to drive LED based on values of buttons
void app_drive_led();


#endif /* INC_APP_H_ */
