/**
 * A good amount of this code came from: https://github.com/NikLeberg/speki/tree/master
 * I also heavily relied on STMicrocontroller's example repo on example code: https://github.com/STMicroelectronics/STM32CubeF4
*/

#include "main.h"

//#include "arm_math.h"



#define STR_NEQ(expected, str) (strncmp(str, expected, sizeof(str)) != 0)
#define STR_EQ(expected, str) (strncmp(str, expected, sizeof(str)) == 0)

#define SONGS_MAX_FATFS_FILE_NAME_LENGTH (sizeof(((FILINFO *)0)->fname))
#define SONGS_MAX_STRING_LENGTH (30U)
#define MAX_NUM_SONGS 3

#define STR_NEQ(expected, str) (strncmp(str, expected, sizeof(str)) != 0)
#define STR_EQ(expected, str) (strncmp(str, expected, sizeof(str)) == 0)
#define DISPLAY_NUM_OF_SPECTOGRAM_BARS (29U)

#define RIGHT 0x02
#define LEFT 0x03
#define CENTER 0x01

#define MARGIN (6U)
#define BOTTOM_STRIP (80U)
#define PROGRESS_BAR (5U)

#define FONT_NORMAL (&Font12)
#define LIST_FONT (FONT_NORMAL)

#define SPECTOGRAM_START_X (0U)
#define SPECTOGRAM_END_X (0x02)

#define SPECTOGRAM_START_Y (0U)
#define SPECTOGRAM_END_Y (CENTER - BOTTOM_STRIP)
#define SPECTOGRAM_HEIGHT (SPECTOGRAM_END_Y - SPECTOGRAM_START_Y)
#define SPECTOGRAM_WIDTH ( \
		(SPECTOGRAM_END_X - SPECTOGRAM_START_X - DISPLAY_NUM_OF_SPECTOGRAM_BARS * MARGIN) / DISPLAY_NUM_OF_SPECTOGRAM_BARS)


#define NAME_FONT (FONT_NORMAL)
#define NAME_START_X (MARGIN)
#define NAME_START_Y (SPECTOGRAM_END_Y + MARGIN)
#define NAME_END_Y (NAME_START_Y + NAME_FONT)



#define ARTIST_FONT (FONT_NORMAL)
#define ARTIST_START_X (NAME_START_X)
#define ARTIST_START_Y (NAME_END_Y + MARGIN)
#define ARTIST_END_Y (ARTIST_START_Y + ARTIST_FONT)

#define PROGRESS_START_X (ARTIST_START_X)
#define PROGRESS_END_X (RIGHT - MARGIN)
#define PROGRESS_START_Y (ARTIST_END_Y + MARGIN)
#define PROGRESS_END_Y (PROGRESS_START_Y + PROGRESS_BAR)

#define PLAY_TIME_FONT (FONT_NORMAL)
#define PLAY_TIME_START_X (PROGRESS_START_X)
#define PLAY_TIME_START_Y (PROGRESS_END_Y + MARGIN)
#define PLAY_TIME_END_Y (PLAY_TIME_START_Y + PLAY_TIME_FONT)



#define USH_USR_FS_INIT       0
#define USH_USR_FS_READLIST   1
//#define USH_USR_FS_WRITEFILE  2
#define USH_USR_FS_PLAY       3

static enum {
	DISPLAY_NOT_INITIALIZED = 0,
	DISPLAY_INITIALIZED,
	DISPLAY_INIT_LIST,
	DISPLAY_LIST,
	DISPLAY_INIT_SONG,
	DISPLAY_SONG
} display_state;

typedef struct __attribute__((packed)) {
    char chunk_id[4];    // id of chunk, "RIFF", "fmt " and "LIST" supported
    uint32_t chunk_size; // size of chunk data, excluding the header (- 8 bytes)
} chunk_header_t;

// chunk id "RIFF"
typedef struct __attribute__((packed)) {
    char format[4]; // "WAVE"
} riff_chunk_t;

// chunk id "fmt "
typedef struct __attribute__((packed)) {
    uint16_t audio_format;    // = 1 for PCM
    uint16_t num_channels;    // Mono = 1, Stereo = 2
    uint32_t sample_rate;     // 8000, 44100, 48000 etc.
    uint32_t byte_rate;       // sample_rate * num_channels * (bits_per_sample / 8)
    uint16_t block_align;     // num_channels * (bits_per_sample / 8)
    uint16_t bits_per_sample; // 8, 16, 24 or 32
} fmt_chunk_t;

// chunk id "LIST"

typedef struct __attribute__((packed)) {
    char format[4]; // "INFO"
} list_chunk_t;

typedef struct {
    FIL file;				// FatFS fp
    char filename[SONGS_MAX_FATFS_FILE_NAME_LENGTH];
    char name[SONGS_MAX_STRING_LENGTH];
    char artist[SONGS_MAX_STRING_LENGTH];
    size_t samples;                                  // num of samples of the full song
    size_t samples_read;                             // num samples already read
} song_t;


// list of all the .wav songs
typedef struct {
	size_t num_songs;
	song_t songs[MAX_NUM_SONGS]; 			// beginning of the songs found
} song_list_t;

typedef enum
{
  APPLICATION_IDLE = 0,
  APPLICATION_START
}
MSC_ApplicationTypeDef;


song_list_t songs;
size_t num_songs = MAX_NUM_SONGS;

USBH_HandleTypeDef  hUSBHost;
FATFS USBDISK_FatFs;
uint8_t USBDISK_Driver_Num;  /* USB Host logical drive number */


FIL wav_file;


MSC_ApplicationTypeDef Appli_state = APPLICATION_IDLE;
uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;

DAC_HandleTypeDef    DacHandle;
static DAC_ChannelConfTypeDef sConfig;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
void Error_Handler(void);
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

static void Toggle_Leds(void);

static void MSC_Application(void);

static int parse_headers(FIL *fp, song_t *song);		// parsing
static int parse_data(FIL *fp, song_t *song);

int init_fs();

int validate_wav_file(FIL *fp, char *fname);
void populate_songs(song_list_t* songs, size_t len);

int display_init();
void display_set_list(song_list_t *curr_songs, size_t ls_len);
int display_loop();
int display_move_selection(int direction);
int display_set_song(const song_t *song);
int display_set_spectogram(uint32_t spectogram[DISPLAY_NUM_OF_SPECTOGRAM_BARS], uint32_t max_value);

static uint8_t explore_disk(char *path);

static void TIM6_Config(void);

int main(void) {


	HAL_Init();
	SystemClock_Config();

	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);

	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);


	display_init();

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

	 DacHandle.Instance = DAC;
	 TIM6_Config();


	while (1) {
		/* USER CODE END WHILE */


		/* USER CODE BEGIN 3 */
		if (Appli_state == APPLICATION_START) {
			MSC_Application();
		}

		USBH_Process(&hUSBHost);
		Toggle_Leds();
	}

}

static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
  switch (id)
  {
  case HOST_USER_DISCONNECTION:
    Appli_state = APPLICATION_IDLE;
    if (f_mount(&USBDISK_FatFs, "", 0) != FR_OK)
    {
      /* FatFs Initialization Error */
      Error_Handler();
    }
    break;

  case HOST_USER_CLASS_ACTIVE:
    Appli_state = APPLICATION_START;
    break;
  }
}

static void MSC_Application(void) {
	switch(USBH_USR_ApplicationState) {
		case USH_USR_FS_INIT:
			/* Register work area for logical drives */
			if (f_mount(&USBDISK_FatFs, "", 0) != FR_OK) {
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
	}

	return;

}
static uint8_t explore_disk(char *path) {
	  FRESULT res;
	  FILINFO fno;
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
static void Toggle_Leds(void)
{
  static uint32_t i;
  if (i++ == 0x10000)
  {
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
int init_fs() {
	return (f_mount(&USBDISK_FatFs, "", 0) != FR_OK);
}

static int parse_headers(FIL *fp, song_t *song) {
    size_t bytes_read = 0;
	chunk_header_t header = {0};

    while (1) {
        // read chunk header
        f_read(fp, &header, sizeof(chunk_header_t), &bytes_read);
        if (bytes_read != sizeof(chunk_header_t) || header.chunk_size == 0) {
            // failed to read data
            return -1;
        }

        if (STR_EQ("LIST", header.chunk_id)) {
            // found the header
            list_chunk_t list = {0};
            f_read(fp, &list, sizeof(list_chunk_t), &bytes_read);
            if (bytes_read != sizeof(list_chunk_t) || STR_NEQ("INFO", list.format)) {
                // failed to read data
                return -1;
            }
            break;
        } else {
            // skip unknown chunk
            header.chunk_size += header.chunk_size % 2;
            f_lseek(fp, fp->fptr + header.chunk_size);
            continue;
        }
    }


    // read chunk header of info (and skip unknown chunks if neccessary)

    while (1) {
        f_read(fp, &header, sizeof(chunk_header_t), &bytes_read);
        if (bytes_read != sizeof(chunk_header_t) || header.chunk_size == 0) {
            // failed to read data
            return -1;
        }

        // name of the artist
        if (STR_EQ("IART", header.chunk_id)) {
            char temp[header.chunk_size];
            f_read(fp, temp, header.chunk_size, &bytes_read);
            strncpy(song->artist, temp, SONGS_MAX_STRING_LENGTH);
            song->artist[SONGS_MAX_STRING_LENGTH - 1] = '\0';
            if (header.chunk_size % 2) {
                f_lseek(fp, fp->fptr + 1);
            }
        } else if (STR_EQ("INAM", header.chunk_id)) {
            // name of the song
            char temp[header.chunk_size];
            f_read(fp, temp, header.chunk_size, &bytes_read);
            strncpy(song->name, temp, SONGS_MAX_STRING_LENGTH);
            song->name[SONGS_MAX_STRING_LENGTH - 1] = '\0';
            if (header.chunk_size % 2) {
                f_lseek(fp, fp->fptr + 1);
            }
        } else if (STR_EQ("data", header.chunk_id)) {
            // We went too far and reached the end of the info chunk. The next
            // chunk is already the pcm data. Rewind the filepointer and exit.
            f_lseek(fp, fp->fptr - sizeof(chunk_header_t));
            break;
        } else {
            // this chunk is unknown, skip it
            header.chunk_size += header.chunk_size % 2;
            f_lseek(fp, fp->fptr + header.chunk_size);
        }
    }

    return 0;
}

static int parse_data(FIL *fp, song_t *song) {
	size_t bytes_read = 0;
	chunk_header_t header = {0};
	while (1) {
		// read chunk header
		f_read(fp, &header, sizeof(chunk_header_t), &bytes_read);
		if (bytes_read != sizeof(chunk_header_t) || header.chunk_size == 0) {
			// failed to read data
			return -1;
		}

		if (STR_NEQ("data", header.chunk_id)) {
			// this chunk is unknown, skip it
			header.chunk_size += header.chunk_size % 2;
			f_lseek(fp, fp->fptr + header.chunk_size);
		} else {
			// we found the data, exit loop
			break;
		}
	}

	song->samples = header.chunk_size / 2;
	// File pointer is now at the end of all headers, what follows is just the
	// raw pcm bitstream.
	return 0;
}

int validate_wav_file(FIL *fp, char *fname) {


	/// reading the default RIFF and WAV format header

	size_t bytes_read = 0;
	chunk_header_t header = {0};
	f_read(fp, &header, sizeof(chunk_header_t), &bytes_read);
	if (STR_NEQ("RIFF", header.chunk_id) || bytes_read != sizeof(chunk_header_t)) {
		return -1;
	}

    // chunk data for id "RIFF"
    //  - should be just "WAVE"
    riff_chunk_t riff = {0};
    f_read(fp, &riff, sizeof(riff_chunk_t), &bytes_read);
    if (STR_NEQ("WAVE", riff.format) || bytes_read != sizeof(riff_chunk_t)) {
    	return -1;
    }

    // read chunk header for format
    //  - should be of id "fmt "
    //  - should have a size of 16 bytes
    f_read(fp, &header, sizeof(chunk_header_t), &bytes_read);
    if (header.chunk_size != sizeof(fmt_chunk_t) || bytes_read != sizeof(chunk_header_t) || STR_NEQ("fmt ", header.chunk_id)) {
    	return -1;
    }

    // chunk data for id "fmt ", should have:
    // - 16 bits depth
    // - 48 kHz sample rate
    // - stereo channel
    // - uncompressed pcm encoding
    fmt_chunk_t fmt = {0};
    f_read(fp, &fmt, sizeof(fmt_chunk_t), &bytes_read);
    if (bytes_read != sizeof(fmt_chunk_t) || fmt.audio_format != 1 || fmt.num_channels != 2 || fmt.sample_rate != 48000 || fmt.bits_per_sample != 16) {
    	return -1;
    }



	return 0;
}


// asumes only .wav files are in the USB stick
void populate_songs(song_list_t* songs, size_t len) {
	// Open root directory to search for .wav files.
	DIR dir;
	FRESULT res;
	char *fn;
	res = f_opendir(&dir, "0:/");
	if (res != FR_OK) {
		return;
	}

	size_t song_nr = 0;


	while(song_nr < len) {	// loop through all the files and populate the
		 FILINFO curr_file;

		 // exit if at the end of directory or on error
		 if (f_readdir(&dir, &curr_file) != FR_OK || curr_file.fname[0] == 0) {
			 break;
		 }


		 fn = curr_file.fname;
		 LCD_UsrLog(fn);
		 LCD_UsrLog("\n");
		 // making sure the filename contains .wav
		 if ((strstr(fn, "wav")) || (strstr(fn, "WAV"))) {
			 FRESULT res = f_open(&wav_file, curr_file.fname, FA_OPEN_EXISTING | FA_READ);
			 if (res != FR_OK) {
				// should proply print an error indicating failure to open wav file but whatever
				continue;
			 }
			 int result = validate_wav_file(&wav_file, curr_file.fname);
			 if (result != -1) {
				 ////// parsing header info
				if (parse_headers(&wav_file, &songs->songs[song_nr]) != 0) {
					// we don't know the artist or song name so set it to unknown
					strncpy(songs->songs[song_nr].artist, "Unknown", SONGS_MAX_STRING_LENGTH);
					strncpy(songs->songs[song_nr].name, "Unknown", SONGS_MAX_STRING_LENGTH);
				}

				// read chunk header for data (and skip unknown chunks if neccessary)
				if (parse_data(&wav_file, &songs->songs[song_nr]) != 0) {
					 f_close(&wav_file);
					continue;
				}
				song_t *song = &songs->songs[song_nr];
				song->file = wav_file;
				song_nr++;
				f_close(&wav_file);
			 } else {
				 // invalid .wav, so silently skip
				 f_close(&wav_file);
				 continue;
			 }
		 } else {
			 // silently skipping non wav files
			 f_close(&wav_file);
			 continue;
		 }
	}

	f_closedir(&dir);

}

int display_init() {
    if (display_state != DISPLAY_NOT_INITIALIZED) {
        // only allow initialization once
        return -1;
    }

	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER, (LCD_FRAME_BUFFER + BUFFER_OFFSET));
	BSP_LCD_SetTransparency(LCD_BACKGROUND_LAYER, 0);
	BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);

	BSP_LCD_Clear(LCD_COLOR_BLACK);
	/* LCD Log initialization */
	LCD_LOG_Init();

	LCD_LOG_SetHeader((uint8_t *)"LETS GO BITCH");
	LCD_UsrLog("> USB Host library started.\n");
	LCD_LOG_SetFooter ((uint8_t *)"     USB Host Library V3.2.0" );
	display_state = DISPLAY_INITIALIZED;
	return 0;
}

void display_set_list(song_list_t *curr_songs, size_t ls_len) {

	BSP_LCD_SetFont(LIST_FONT);
    for (int i = 0; i < ls_len; ++i) {
        char tmp[(SONGS_MAX_STRING_LENGTH * 2) + 3]; // "artist" + " - " + "title"
        snprintf(tmp, sizeof(tmp), "%s - %s", curr_songs->songs[i].artist, curr_songs->songs[i].name);
        strcat(tmp, "\n");
        LCD_UsrLog(tmp);
    }
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    display_state = DISPLAY_INIT_LIST;
}


void TIM6_Config(void)
{
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
