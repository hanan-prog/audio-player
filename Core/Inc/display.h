#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"

#include  "../../Utilities/Log/lcd_log.h"
#include "audio_fs.h"

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


int display_init(song_list_t *curr_songs, size_t ls_len);

int display_update(void);
void display_move_selection(void);

int display_set_song(const song_t *song);

int display_set_spectogram(uint32_t spectogram[DISPLAY_NUM_OF_SPECTOGRAM_BARS], uint32_t max_value);

#endif // DISPLAY_H
