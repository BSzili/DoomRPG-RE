#include "SDL.h"

#include <stdarg.h>

static char errorstr[64];

int SDL_SetError(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(errorstr, sizeof(errorstr), fmt, ap);
	va_end(ap);
	return -1;
}

const char* SDL_GetError(void)
{
	return errorstr;
}

void SDL_ClearError(void)
{
	errorstr[0] = '\0';
}

/*
void SDL_Log(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}
*/

