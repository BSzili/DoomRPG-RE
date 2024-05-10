#include "SDL.h"
#include "SDL_mixer.h"

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/ahi.h>
#include <devices/ahi.h>

#include <clib/debug_protos.h>

#include <SDI_compiler.h>
#include <SDI_hook.h>

#include "audiodev.h"

#define MAX_CHANNELS (2) // MAX_SOUNDCHANNELS 10
#define MAX_SOUNDS (100) // Doom RPG has 92 sounds
#define SOUND_SAMPLERATE 8000

struct Library *AHIBase = NULL;
static struct MsgPort *AHImp = NULL;
static struct AHIRequest *AHIio = NULL;
static struct AHIAudioCtrl *actrl = NULL;
//static struct Hook SoundHook;
static BYTE AHIDevice = -1;
//static ULONG audioID = AHI_DEFAULT_ID;
//static int SOUND_SAMPLERATE = 11025;
static BOOL audioDevice = FALSE;

static WORD ChannelPlaying[MAX_CHANNELS];
static UWORD NextSound = 0;


#ifndef Mix_OpenAudio
int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
{
	//printf("%s(%d,%u,%d,%d)\n", __FUNCTION__, frequency, format, channels, chunksize);

	if ((AHImp = CreateMsgPort())) {
		if ((AHIio = (struct AHIRequest *)CreateIORequest(AHImp, sizeof(struct AHIRequest)))) {
			AHIio->ahir_Version = 4;
			if (!(AHIDevice = OpenDevice((STRPTR)AHINAME, AHI_NO_UNIT, (struct IORequest *)AHIio, 0))) {
				AHIBase = (struct Library *)AHIio->ahir_Std.io_Device;

				char namebuf[64];

				AHI_GetAudioAttrs(AHI_DEFAULT_ID, NULL,
					AHIDB_BufferLen, sizeof(namebuf),
					AHIDB_Driver, (IPTR)namebuf,
					TAG_END);

				if (strcmp(namebuf, "paula")) {
					AHI_GetAudioAttrs(AHI_DEFAULT_ID, NULL,
						AHIDB_BufferLen, sizeof(namebuf),
						AHIDB_Name, (IPTR)namebuf,
						TAG_END);

					printf("%s using AHI mode: %s\n", __FUNCTION__, namebuf);

					return 0;
				}

				CloseDevice((struct IORequest *)AHIio);
				AHIDevice = -1;
				AHIBase = NULL;
			}
			DeleteIORequest((struct IORequest *)AHIio);
			AHIio = NULL;
		}
		DeleteMsgPort(AHImp);
		AHImp = NULL;
	}

	if (sound_init(SOUND_SAMPLERATE)) {
		audioDevice = TRUE;
		printf("%s using audio.device\n", __FUNCTION__);
		return 0;
	}

	return -1;
}
#endif

static unsigned int log2base(unsigned int v)
{
	unsigned int r = 0;
	while (v >>= 1) r++;
	return r;
}

HOOKPROTO(SoundFunc, ULONG, struct AHIAudioCtrl *actrl, struct AHISoundMessage *smsg)
{
	ChannelPlaying[smsg->ahism_Channel] = FALSE;
	return 0;
}
MakeHook(SoundHook, SoundFunc);

#ifndef Mix_AllocateChannels
int Mix_AllocateChannels(int numchans)
{
	//printf("%s(%d)\n", __FUNCTION__, numchans);

	if (AHIBase) {
		ULONG ahiChannels = numchans;
		if (ahiChannels >= MAX_CHANNELS) ahiChannels = MAX_CHANNELS;
		ULONG ahiSounds = MAX_SOUNDS;

		actrl = AHI_AllocAudio(
			AHIA_MixFreq, SOUND_SAMPLERATE,
			AHIA_Channels, ahiChannels,
			AHIA_Sounds, ahiSounds,
			AHIA_SoundFunc, (IPTR)&SoundHook,
			TAG_DONE);

		if (actrl) {
			ULONG r = log2base(ahiChannels);
			struct AHIEffMasterVolume vol = {
				AHIET_MASTERVOLUME,
				r * 0x10000
			};
			AHI_SetEffect(&vol, actrl);

			AHI_ControlAudio(actrl, AHIC_Play, TRUE, TAG_END);

			return numchans;
		}
	}

	return 0;
}
#endif

#ifndef Mix_Quit
void Mix_Quit(void)
{
	if (actrl) {
		AHI_ControlAudio(actrl, AHIC_Play, FALSE, TAG_END);
		struct AHIEffMasterVolume vol = {
			AHIET_MASTERVOLUME | AHIET_CANCEL,
			0x10000
		};
		AHI_SetEffect(&vol, actrl);
		AHI_FreeAudio(actrl);
		actrl = NULL;
	}

	if (!AHIDevice) {
		CloseDevice((struct IORequest *)AHIio);
		AHIDevice = -1;
		AHIBase = NULL;
	}

	if (AHIio) {
		DeleteIORequest((struct IORequest *)AHIio);
		AHIio = NULL;
	}

	if (AHImp) {
		DeleteMsgPort(AHImp);
		AHImp = NULL;
	}

	if (audioDevice) {
		audioDevice = FALSE;
		//sound_isplaying();
		//sound_stop();
		sound_close();
	}
}
#endif

#ifndef Mix_Volume
int Mix_Volume(int channel, int volume)
{
	// always called with -1 to set the global volume
	return MIX_MAX_VOLUME; // TODO
}
#endif

#ifndef Mix_HaltChannel
int Mix_HaltChannel(int channel)
{
	//printf("%s(%d)\n", __FUNCTION__, channel);

	if (actrl) {
		if (channel >= MAX_CHANNELS) {
			return -1;
		}
		AHI_SetSound(channel, AHI_NOSOUND, 0, 0, actrl, AHISF_IMM);
		ChannelPlaying[channel] = FALSE;
	}

	if (audioDevice) {
		if (channel >= 2) {
			return -1;
		}
		//if (sound_isplaying()) {
			//sound_stop();
		//}
	}

	return 0;
}
#endif

#ifndef Mix_PlayChannel
int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops)
{
	//printf("%s(%d,%p,%d) sound %d length %d\n", __FUNCTION__, channel, chunk, loops, chunk->allocated, chunk->alen);

	if (actrl) {
		if (channel >= MAX_CHANNELS) {
			return -1;
		}

		const int freq = SOUND_SAMPLERATE; // TODO don't hardcode
		int vol = chunk->volume << 9;

		AHI_SetFreq(channel, freq, actrl, AHISF_IMM);
		AHI_SetVol(channel, vol, 0x8000, actrl, AHISF_IMM);
		AHI_SetSound(channel, chunk->allocated, 0, chunk->alen, actrl, AHISF_IMM);
		if (loops == 0) {
			AHI_SetSound(channel, AHI_NOSOUND, 0, 0, actrl, AHISF_NONE);
		}

		ChannelPlaying[channel] = TRUE;
	}

	if (audioDevice) {
		if (channel >= 2) {
			return -1;
		}
		int vol = chunk->volume >> 1;
		sound_play(channel, chunk->abuf, chunk->alen, vol, vol, loops + 1);
	}

	return 0; // TODO channel number?
}
#endif

#ifndef Mix_Playing
int Mix_Playing(int channel)
{
	//printf("%s(%d)\n", __FUNCTION__, channel);

	if (actrl) {
		if (channel >= MAX_CHANNELS) {
			return 1; // pretend to be playing so the game won't try to use the channel
		}
		return ChannelPlaying[channel];
	}

	if (audioDevice) {
		if (channel >= 2) {
			return 1;
		}
#if 0
		int playing = sound_isplaying();
		//printf("%s playing %d\n", __FUNCTION__, playing);
		return playing;
#else
		return sound_isplaying(channel);
#endif
	}
	return 0;
}
#endif

#ifndef Mix_VolumeChunk
int Mix_VolumeChunk(Mix_Chunk *chunk, int volume)
{
	//printf("%s(%p,%d)\n", __FUNCTION__, chunk, volume);

	chunk->volume = volume;

	return MIX_MAX_VOLUME; // TODO not used?
}
#endif

#ifndef Mix_FreeChunk
void Mix_FreeChunk(Mix_Chunk *chunk)
{
	if (chunk) {
		if (actrl) {
			AHI_UnloadSound(chunk->allocated, actrl);
		}
		if (chunk->abuf) {
			if (audioDevice) {
				FreeVec(chunk->abuf);
			} else {
				free(chunk->abuf);
			}
			chunk->abuf = NULL;
		}
		free(chunk);
	}
}
#endif

#ifndef Mix_LoadWAV_RW
Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc)
{
	SDL_bool was_error = SDL_FALSE;
	Mix_Chunk *chunk = NULL;
	char name[4];
	int size;

	//printf("%s(%p,%d)\n", __FUNCTION__, src, freesrc);

	SDL_RWread(src, name, 4, 1);
	if (memcmp(name, "RIFF", 4)) {
		Mix_SetError("RIFF chunk not found");
		was_error = SDL_TRUE;
		goto done;
	}
	size = SDL_ReadLE32(src);
	//printf("%s RIFF size %d\n", __FUNCTION__, size);

	SDL_RWread(src, name, 4, 1);
	if (memcmp(name, "WAVE", 4)) {
		Mix_SetError("WAVE chunk not found");
		was_error = SDL_TRUE;
		goto done;
	}
	SDL_RWread(src, name, 4, 1);
	if (memcmp(name, "fmt ", 4)) {
		Mix_SetError("fmt chunk not found");
		was_error = SDL_TRUE;
		goto done;
	}
	size = SDL_ReadLE32(src);
	//printf("%s fmt size %d\n", __FUNCTION__, size);

	int AudioFormat = SDL_ReadLE16(src);
	if (AudioFormat != 1) {
		Mix_SetError("unsupported audio format");
		was_error = SDL_TRUE;
		goto done;
	}
	int NumChannels = SDL_ReadLE16(src);
	int SampleRate = SDL_ReadLE32(src);
	int ByteRate = SDL_ReadLE32(src);
	int BlockAlign = SDL_ReadLE16(src);
	int BitsPerSample = SDL_ReadLE16(src);
	//printf("%s format %d channels %d rate %d bits %d\n", __FUNCTION__, AudioFormat, NumChannels, SampleRate, BitsPerSample);
	if (SampleRate != 8000 || NumChannels != 1 || BitsPerSample != 16) {
		Mix_SetError("only 8 KHz 16-bit mono waves are supported");
		was_error = SDL_TRUE;
		goto done;
	}

	SDL_RWread(src, name, 4, 1);
	if (memcmp(name, "data", 4)) {
		Mix_SetError("data chunk not found");
		was_error = SDL_TRUE;
		goto done;
	}
	size = SDL_ReadLE32(src);
	//printf("%s data size %d\n", __FUNCTION__, size);
	int samples = size >> 1;

	chunk = calloc(sizeof(Mix_Chunk)/* + samples*/, 1);
	if (chunk) {
		chunk->allocated = NextSound++;
		//chunk->abuf = (Uint8 *)(chunk + 1);
		if (audioDevice) {
			const int endsamples = 4; // audio.device bug workaround
			chunk->abuf = AllocVec(samples + endsamples, MEMF_CLEAR|MEMF_CHIP);
			chunk->alen = samples + endsamples;
		} else {
			chunk->abuf = calloc(samples, 1);
			chunk->alen = samples;
		}
		chunk->volume = MIX_MAX_VOLUME;

		Sint8 *dest = (Sint8 *)chunk->abuf;
		for (int i = 0; i < samples; i++) {
			Sint16 sample = SDL_ReadLE16(src);
			*dest++ = sample >> 8;
		}

#if 0
		{
			// dump the sample as a Sun AU file
			char filename[256];
			FILE *fd;
			struct
			{
				uint32_t magic; /* magic number */
				uint32_t hdr_size; /* size of this header */ 
				uint32_t data_size; /* length of data (optional) */ 
				uint32_t encoding; /* data encoding format */
				uint32_t sample_rate; /* samples per second */
				uint32_t channels; /* number of interleaved channels */
			} header;
			header.magic = SDL_SwapBE32(0x2e736e64);
			header.hdr_size = SDL_SwapBE32(sizeof(header));
			header.data_size = SDL_SwapBE32(0xffffffff);
			header.encoding = SDL_SwapBE32(2);
			header.sample_rate = SDL_SwapBE32(SampleRate);
			header.channels = SDL_SwapBE32(NumChannels);

			snprintf(filename, sizeof(filename), "sound%02d.au", chunk->allocated);
			fd = fopen(filename, "w");
			fwrite(&header, sizeof(header), 1, fd);
			fwrite(chunk->abuf, chunk->alen, 1, fd);
			fclose(fd);
		}
#endif

		if (actrl) {
			ULONG error;
			struct AHISampleInfo sample;

			sample.ahisi_Address = chunk->abuf;
			sample.ahisi_Type = AHIST_M8S; //AHIST_M16S;
			sample.ahisi_Length = chunk->alen;
			error = AHI_LoadSound(chunk->allocated, AHIST_SAMPLE, &sample, actrl);
			// TODO error check

			//printf("%s loaded sound %2d length %d error %lu\n", __FUNCTION__, chunk->allocated, chunk->alen, error);
		}
	}

done:
	/*
	if (was_error) {
		Mix_FreeChunk(chunk);
		chunk = NULL;
	}
	*/
	if (freesrc && src) {
		SDL_RWclose(src);
	}
	return chunk;
}
#endif
