#ifndef NOTSDL_MIXER_H
#define NOTSDL_MIXER_H

/*
typedef struct Mix_Chunk
{
	void *handle;
} Mix_Chunk;
*/
typedef struct Mix_Chunk
{
	int allocated;
	Uint8 *abuf;
	Uint32 alen;
	Uint8 volume;       /* Per-sample volume, 0-128 */
} Mix_Chunk;

#define MIX_DEFAULT_FORMAT (0) // hack
#define MIX_MAX_VOLUME 128


#if 1
int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);
#define Mix_SetError SDL_SetError
#define Mix_GetError SDL_GetError
#define Mix_ClearError SDL_ClearError
/*
int Mix_SetError(const char *fmt, ...);
const char *Mix_GetError(void);
void Mix_ClearError(void);
*/
void Mix_Quit(void);
int Mix_AllocateChannels(int numchans);
int Mix_Volume(int channel, int volume);
void Mix_FreeChunk(Mix_Chunk *chunk);
int Mix_HaltChannel(int channel);
int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops);
int Mix_Playing(int channel);
int Mix_VolumeChunk(Mix_Chunk *chunk, int volume);
#else
#define Mix_OpenAudio(x,y,z,w) (0) // pretend to be working
#define Mix_LoadWAV_RW(x,y) NULL
#define Mix_GetError() NULL
#define Mix_Quit()
#define Mix_AllocateChannels(x)
#define Mix_Volume(x,y)
#define Mix_FreeChunk(x)
#define Mix_HaltChannel(x)
#define Mix_PlayChannel(x,y,z)
#define Mix_Playing(x) (0)
#define Mix_VolumeChunk(x,y)
#endif

#endif
