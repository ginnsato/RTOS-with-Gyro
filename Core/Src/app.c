/**
 * @file app.c
 *
 * @author
 *  Ginn Sato
 *
 * @date
 *  01/31/2023
 *
 * @brief
 *  Defines application functions to handle gyro sampling and driving LEDs.
 *
 */

#include <app.h>

GPIO_PinState USER_BUTTON_STATE;
GYRO_DIRECTION GYRO_DIR;

static Data_Ready_t data_ready = READY;
static int16_t old_data = 0;


#if!(LAB3_USE_TASK)
/***************************************************************************//**
* @brief
*  Callback function for timer 1
*
* @details
*  Handles each timer 1 callback by driving LEDs and sampling gyro.
*
* @retval None
*
******************************************************************************/
void Timer1_Callback(void *arg){
	(void) &arg;
	// Sample gyro and drive LEDs. Occurs every 100ms by configuration.
	app_get_gyro_rate();
	app_drive_led();
}


/***************************************************************************//**
* @brief
*  Operating System Timer Initialization
*
* @details
*  Initialize a new timer.
*
* @retval None
*
******************************************************************************/
void osTimer_Init(){
	static osTimerId_t timer_id;
	static StaticTimer_t timer_tcb;
	const osTimerAttr_t timer_attr = {
			.name = "timer1",
			.cb_mem = &timer_tcb,
			.cb_size = sizeof(timer_tcb),
	};

	// Create a timer and ensure it was successful
	timer_id = osTimerNew(Timer1_Callback, osTimerPeriodic, NULL, &timer_attr);
	if(timer_id == NULL)
		while(1);

	// Start timer and ensure it was successful
	osStatus_t status = osTimerStart(timer_id, 100U);
	if(status != osOK)
		while(1);
}
#endif


#if(LAB3_USE_TASK)
/***************************************************************************//**
* @brief
*  Handler function for task 1
*
* @details
*  Handles task 1 by get gyro rate and driving LEDs.
*
* @retval None
*
******************************************************************************/
void task1 (void *arg) {
	(void) &arg;

	// Sample gyro, drive LEDs, and delay 100ms
	while(1){
		app_get_gyro_rate();
		app_drive_led();
		osStatus_t status = osDelay(100);
		if(status != osOK)
			while(1);
	}
}


/***************************************************************************//**
* @brief
*  Operating System Thread Initialization
*
* @details
*  Initialize a new thread and ensures successful creation.
*
* @retval None
*
******************************************************************************/
void osThread_Init(){
	static StaticTask_t task1_tcb;
	static uint32_t task1_stack[64];
	static osThreadId_t task1_id;

	const osThreadAttr_t task1_attr = {
			.name = "task1",
			.cb_mem = &task1_tcb,
			.cb_size = sizeof(task1_tcb),
			.stack_mem = &task1_stack[0],
			.stack_size = sizeof(task1_stack),
			.priority = osPriorityNormal,
	};

	// create a task/thread and ensure it was successful
	task1_id = osThreadNew(task1, NULL, &task1_attr);
	if(task1_id == NULL)
		while(1);
}
#endif


/***************************************************************************//**
* @brief
*  Application Initialization
*
* @details
*  Initialize hardware we are going to use in the application.
*
* @retval None
*
******************************************************************************/
void app_init(){
	HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
	Gyro_Init();

	// Enable button interrupt
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	// Enable GYRO interrupt
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);

	#if!(LAB3_USE_TASK)
	osTimer_Init();
	#endif

	#if(LAB3_USE_TASK)
	osThread_Init();
	#endif

}


/***************************************************************************//**
* @brief
*  Sample User Button
*
* @details
* 	Set global variable that keeps track of button state.
*
* @retval None
*
******************************************************************************/
void app_sample_user_button(){
	// set global variable
	USER_BUTTON_STATE = HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN);
}


/***************************************************************************//**
* @brief
*  Get gyro rate
*
* @details
*  Check if new data is ready. If so get the gyro velocity, and if not we reuse
*  the old value. From whatever value we get, determine if its CW or CCW.
*
* @retval None
*
******************************************************************************/
void app_get_gyro_rate(){
	int16_t gyro_vel;//RE = Gyro_Get_Velocity();;

	// Check if new data is ready and handle accordingly
	if(data_ready == NOT_READY)
		gyro_vel = old_data;
	else
		 gyro_vel = Gyro_Get_Velocity();

	// Reset data ready variable since we just read
	data_ready = NOT_READY;

	// Set old data to what we just read
	old_data = gyro_vel;

	// Categorize as either CW or CCW
	if(gyro_vel > -5000)
		GYRO_DIR = CW;
	else
		GYRO_DIR = CCW;
}


/***************************************************************************//**
* @brief
*  Drive the LEDs
*
* @details
* 	Drive the LEDs based on button states and gyro state
*
* @retval None
*
******************************************************************************/
void app_drive_led(){

	// Drive green LED: while button is pressed or while gyro is rotating
	if(USER_BUTTON_STATE == GPIO_PIN_SET || GYRO_DIR == CCW)
		HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);

	// Drive red LED: while button is pressed and gyro is rotating clockwise
	if(USER_BUTTON_STATE == GPIO_PIN_SET && GYRO_DIR == CW)
		HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);

}


/***************************************************************************//**
* @brief
*  EXTI0 Interrupt Handler
*
* @details
* 	Handle the EXTI0 interrupts. Samples the user button on button press.
*
* @retval None
*
******************************************************************************/
void EXTI0_IRQHandler(){
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);

	app_sample_user_button();

	__HAL_GPIO_EXTI_CLEAR_FLAG(USER_BUTTON_PIN);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}


/***************************************************************************//**
* @brief
*  EXTI2 Interrupt Handler
*
* @details
* 	Handle the EXTI2 interrupts. Set global variable indicating new data is ready.
*
* @retval None
*
******************************************************************************/
void EXTI2_IRQHandler(){
	HAL_NVIC_DisableIRQ(EXTI2_IRQn);

	data_ready = READY;

	__HAL_GPIO_EXTI_CLEAR_FLAG(GYRO_DRDY_PIN);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}
