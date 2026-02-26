#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wave.h"

namespace GX
{
	void get_wav_header(int raw_sz, wav_header_t* wh)
	{
		// RIFF chunk
		strcpy((char*)wh->chunk_id, "RIFF");
		wh->chunk_size = 36 + raw_sz;

		// fmt sub-chunk (to be optimized)
		strncpy((char*)wh->sub_chunk1_id, "WAVEfmt ", strlen("WAVEfmt "));
		wh->sub_chunk1_size = 16;
		wh->audio_format = 3;
		wh->num_channels = 2;
		wh->sample_rate = 44100;
		wh->bits_per_sample = 32;
		wh->block_align = wh->num_channels * wh->bits_per_sample / 8;
		wh->byte_rate = wh->sample_rate * wh->num_channels * wh->bits_per_sample / 8;

		// data sub-chunk
		strncpy((char*)wh->sub_chunk2_id, "data", strlen("data"));
		wh->sub_chunk2_size = raw_sz;
	}

	void pcm_to_wav(char* pcm_buf, int buf_size, int channels, int sample_rate, int bps, const char* out_file)
	{
		FILE* fwav;
		wav_header_t wheader;

		memset(&wheader, '\0', sizeof(wav_header_t));

		// construct wav header
		get_wav_header(buf_size, &wheader);

		wheader.sample_rate = sample_rate;
		wheader.num_channels = channels;
		wheader.bits_per_sample = bps;

		// write out the .wav file
		fwav = fopen(out_file, "wb");
		fwrite(&wheader, 1, sizeof(wheader), fwav);
		fwrite(pcm_buf, 1, buf_size, fwav);
		fclose(fwav);
	}
}