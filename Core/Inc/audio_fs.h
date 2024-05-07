#ifndef AUDIO_FS_H
#define AUDIO_FS_H


#include "stm32f429i_discovery.h"

#include "stm32f429i_discovery_lcd.h"

#include  "../../Utilities/Log/lcd_log.h"
#include "ff_gen_drv.h"
#include "usbh_diskio_dma.h"
#include "stdlib.h"



#define SONGS_MAX_FATFS_FILE_NAME_LENGTH 30U
#define SONGS_MAX_STRING_LENGTH 30U
#define MAX_NUM_SONGS 3




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


int unmount_fs(void);

int init_fs(void);

int validate_wav_file(FIL *fp, char *fname);

void populate_songs(song_list_t* songs, size_t len);

#endif // AUDIO_FS_H
