
#include "display.h"

static enum {
	DISPLAY_NOT_INITIALIZED = 0,
	DISPLAY_INITIALIZED,
	DISPLAY_INIT_LIST,
	DISPLAY_LIST,
	DISPLAY_INIT_SONG,
	DISPLAY_SONG
} display_state;


int display_init(void) {
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


int display_loop(void) {
  return 0;
}


int display_move_selection(int direction) {
  return 0;
}


int display_set_song(const song_t *song) {
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


int display_set_spectogram(uint32_t spectogram[DISPLAY_NUM_OF_SPECTOGRAM_BARS], uint32_t max_value) {
	return 0;
}
