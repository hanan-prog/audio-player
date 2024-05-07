#include "audio_fs.h"




#define STR_NEQ(expected, str) (strncmp(str, expected, sizeof(str)) != 0)
#define STR_EQ(expected, str) (strncmp(str, expected, sizeof(str)) == 0)


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


static int parse_headers(FIL *fp, song_t *song);		// parsing
static int parse_data(FIL *fp, song_t *song);

FIL wav_file;

FATFS USBDISK_FatFs;


int unmount_fs(void) {
	return f_mount(&USBDISK_FatFs, "", 0);
}
int init_fs(void) {
	return f_mount(&USBDISK_FatFs, "", 0);
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
  if (bytes_read != sizeof(fmt_chunk_t) || fmt.audio_format != 1 || fmt.num_channels != 1 || fmt.sample_rate != 44000 || fmt.bits_per_sample != 16) {
    return -1;
  }

	return 0;
}


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
