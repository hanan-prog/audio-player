
#include "display.h"

static enum {
	DISPLAY_NOT_INITIALIZED = 0,
	DISPLAY_INITIALIZED,
	DISPLAY_INIT_LIST,
	DISPLAY_LIST,
	DISPLAY_INIT_SONG,
	DISPLAY_SONG
} display_state;


static song_list_t *curr_song_ls;
static size_t song_ls_len;
static size_t curr_song;


static void display_set_list(song_list_t *curr_songs, size_t ls_len);

int display_init(song_list_t *curr_songs, size_t ls_len) {
	if (display_state != DISPLAY_NOT_INITIALIZED) {
		// only allow initialization once
		return -1;
	}

	curr_song_ls = curr_songs;
	song_ls_len = ls_len;
	curr_song = 0;
	display_state = DISPLAY_INITIALIZED;
	display_set_list(curr_song_ls, song_ls_len);

  return 0;
}


int display_update(void) {
	display_set_list(curr_song_ls, song_ls_len);
//	LCD_LOG_Init();		// resetting the display
//	LCD_LOG_SetHeader((uint8_t *)"Update just dropped");
	return 0;
}


void display_move_selection(void) {
	curr_song++;

	if (curr_song > 2) {
		curr_song = 0;
	}
}


int display_set_song(const song_t *song) {
  return 0;
}


static void display_set_list(song_list_t *curr_songs, size_t ls_len) {
	char tmp[(SONGS_MAX_STRING_LENGTH * 2) + 3]; // "artist" + " - " + "title"
	BSP_LCD_SetFont(LIST_FONT);
	for (int i = 0; i < ls_len; ++i) {
		if (i == curr_song) {
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
		} else {
			BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
			BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		}
		snprintf(tmp, sizeof(tmp), "%s - %s", curr_songs->songs[i].artist, curr_songs->songs[i].name);
		strcat(tmp, "\n");
		LCD_UsrLog(tmp);
	}

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
	display_state = DISPLAY_INIT_LIST;
}


int display_set_spectogram(uint32_t spectogram[DISPLAY_NUM_OF_SPECTOGRAM_BARS], uint32_t max_value) {
	return 0;
}
