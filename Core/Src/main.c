/**
 * A good amount of this code came from: https://github.com/NikLeberg/speki/tree/master
 * I also heavily relied on STMicrocontroller's example repo on example code: https://github.com/STMicroelectronics/STM32CubeF4
*/

#include "main.h"







#define USH_USR_FS_INIT       0
#define USH_USR_FS_READLIST   1
#define USH_USR_FS_PLAY       3


typedef enum {
  APPLICATION_IDLE = 0,
  APPLICATION_START
} audio_player_application_t;


song_list_t songs;
size_t num_songs = MAX_NUM_SONGS;

USBH_HandleTypeDef  hUSBHost;
uint8_t USBDISK_Driver_Num;  /* USB Host logical drive number */





audio_player_application_t Appli_state = APPLICATION_IDLE;
uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;

DAC_HandleTypeDef    DacHandle;

//static DAC_ChannelConfTypeDef sConfig;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
void Error_Handler(void);
static void Toggle_Leds(void);

static void USBH_init(void);
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

static void TIM6_Config(void);

static uint8_t explore_disk(char *path);
static void audio_player_application(void);




int main(void) {
	HAL_Init();
	SystemClock_Config();

	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);

	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);


	display_init();
	USBH_init();


	DacHandle.Instance = DAC;
	TIM6_Config();


	while (1) {
		
		if (Appli_state == APPLICATION_START) {
			audio_player_application();
		}

		USBH_Process(&hUSBHost);
		Toggle_Leds();
	}

}

static void USBH_init(void) {
	/* Link the USB Host disk I/O driver */
	USBDISK_Driver_Num = FATFS_LinkDriver(&USBH_Driver, "");

	/* Init Host Library */
	if (USBH_Init(&hUSBHost, USBH_UserProcess, 0) != USBH_OK) {
		/* USB Initialization Error */
		Error_Handler();
	}

	/* Add Supported Class */
	USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

	/* Start Host Process */
	if (USBH_Start(&hUSBHost) != USBH_OK) {
		/* USB Initialization Error */
		Error_Handler();
	}
}

static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id) {
  switch (id) {
  	case HOST_USER_DISCONNECTION:
			Appli_state = APPLICATION_IDLE;
			if (unmount_fs() != FR_OK) {
				/* FatFs Initialization Error */
				Error_Handler();
			}
    	break;

  	case HOST_USER_CLASS_ACTIVE:
			Appli_state = APPLICATION_START;
			break;
  }
}

static void audio_player_application(void) {
	switch(USBH_USR_ApplicationState) {
		case USH_USR_FS_INIT:
			/* Register work area for logical drives */
			if (init_fs() != FR_OK) {
				LCD_ErrLog("> File System NOT initialized.\n");
				Error_Handler();
			} else {
				LCD_UsrLog("> File System initialized.\n");
				USBH_USR_ApplicationState = USH_USR_FS_READLIST;
			}
			break;

		case USH_USR_FS_READLIST:
			LCD_UsrLog("> Exploring disk flash ...\n");

			if (explore_disk("0:/") != FR_OK) {
				LCD_ErrLog("> File cannot be explored.\n");
				Error_Handler();
			} else {
				USBH_USR_ApplicationState = USH_USR_FS_PLAY;
				LCD_UsrLog("To start playing a song\n");
				LCD_UsrLog("Press Key\n");
				while(BSP_PB_GetState (BUTTON_KEY) != SET)
				{
					Toggle_Leds();
				}
			}
			break;
		case USH_USR_FS_PLAY:
			while(BSP_PB_GetState (BUTTON_KEY) != RESET) {
				Toggle_Leds();
				}

				break;
		default:
			break;
	} // end switch

	return;
}

static uint8_t explore_disk(char *path) {
	FRESULT res;
	DIR dir;
	res = f_opendir(&dir, path);

	if (res == FR_OK) {
		populate_songs(&songs, num_songs);
		LCD_LOG_Init();		// resetting the display
		LCD_LOG_SetHeader((uint8_t *)"Pick a song: ");
		display_set_list(&songs, num_songs);
	}
	return res;
}

static void Toggle_Leds(void) {
  static uint32_t i;
  if (i++ == 0x10000) {
    BSP_LED_Toggle(LED3);
    BSP_LED_Toggle(LED4);
    i = 0;
  }
}


/* configure clock 180 MHZ */
static void SystemClock_Config(void) {
	  RCC_ClkInitTypeDef RCC_ClkInitStruct;
	  RCC_OscInitTypeDef RCC_OscInitStruct;

	  /* Enable Power Control clock */
	  __HAL_RCC_PWR_CLK_ENABLE();

	  /* The voltage scaling allows optimizing the power consumption when the device is
	     clocked below the maximum system frequency, to update the voltage scaling value
	     regarding system frequency refer to product datasheet.  */
	  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	  /* Enable HSE Oscillator and activate PLL with HSE as source */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	  RCC_OscInitStruct.PLL.PLLM = 8;
	  RCC_OscInitStruct.PLL.PLLN = 360;
	  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	  RCC_OscInitStruct.PLL.PLLQ = 7;
	  HAL_RCC_OscConfig(&RCC_OscInitStruct);

	  /* Activate the Over-Drive mode */
	  HAL_PWREx_EnableOverDrive();

	  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	     clocks dividers */
	  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}



/* USER CODE BEGIN 4 */








// asumes only .wav files are in the USB stick







void TIM6_Config(void) {
  static TIM_HandleTypeDef htim;
  TIM_MasterConfigTypeDef  sMasterConfig;

  /*##-1- Configure the TIM peripheral #######################################*/
  /* Time base configuration */
  htim.Instance = TIM6;

  htim.Init.Period = 0x7FF;
  htim.Init.Prescaler = 0;
  htim.Init.ClockDivision = 0;
  htim.Init.CounterMode = TIM_COUNTERMODE_UP;
  HAL_TIM_Base_Init(&htim);

  /* TIM6 TRGO selection */
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

  HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig);

  /*##-2- Enable TIM peripheral counter ######################################*/
  HAL_TIM_Base_Start(&htim);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* Turn LED4 on */
  BSP_LED_On(LED4);
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
