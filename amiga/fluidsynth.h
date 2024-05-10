#ifndef NOTFLUIDSYNTH_H
#define NOTFLUIDSYNTH_H

typedef struct fluid_player_s
{
	void *dummy;
} fluid_player_t;

typedef struct fluid_settings_s
{
	void *dummy;
} fluid_settings_t;

typedef struct fluid_synth_s
{
	void *dummy;
} fluid_synth_t;

typedef struct fluid_audio_driver_s
{
	void *dummy;
} fluid_audio_driver_t;

#define FLUID_PLAYER_PLAYING 1

#define new_fluid_settings(x) ((void*)1)
#define new_fluid_synth(x) ((void*)1)
#define new_fluid_audio_driver(x,y) ((void*)1)
#define new_fluid_player(x) NULL


#define delete_fluid_audio_driver(x)
#define delete_fluid_synth(x)
#define delete_fluid_player(x)
#define delete_fluid_settings(x)

#define fluid_player_get_status(x) (0)
#define fluid_player_stop(x)
#define fluid_player_seek(x,y)
#define fluid_settings_setnum(x,y,z)
#define fluid_is_soundfont(x) (1)
#define fluid_synth_sfload(x,y,z)
#define fluid_player_add_mem(x,y,z)
#define fluid_player_set_loop(x, y)
#define fluid_player_play(x)

#endif
