#include "SDL.h"

#include <time.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <devices/timer.h>
#include <sys/time.h>

#define TimerBase DOSBase->dl_TimeReq->tr_node.io_Device

static struct timeval start;

int SDL_TimerInit(void)
{
	GetSysTime(&start);
	return 0;
}

void SDL_TimerQuit(void)
{
}

Uint32 SDL_GetTicks(void)
{
	struct timeval now;
	Uint32 ticks;

	GetSysTime(&now);
	//ticks = (now.tv_sec * 1000) + (now.tv_usec / 1000);
	ticks = ((now.tv_sec - start.tv_sec) * 1000) + ((now.tv_usec - start.tv_usec) / 1000);
	return ticks;
}
