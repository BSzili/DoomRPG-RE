#ifndef _AUDIODEV_H
#define _AUDIODEV_H

int sound_init(int rate);
void sound_close(void);
void sound_stop(int channel);
//void sound_setvolume(int leftvol, int rightvol);
int sound_isplaying(int channel);
void sound_play(int channel, char *data, int length, int leftvol, int rightvol, int cycles);

#endif
