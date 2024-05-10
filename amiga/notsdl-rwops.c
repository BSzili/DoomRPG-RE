#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>

SDL_RWops *SDL_RWFromMem(void *mem, int size)
{
	if (!mem || !size)
	{
		SDL_SetError("Invalid arguments.");
		return NULL;
	}

	SDL_RWops *rwops = calloc(1, sizeof(SDL_RWops));
	if (rwops)
	{
		rwops->hidden.mem.base = (Uint8 *)mem;
		rwops->hidden.mem.here = rwops->hidden.mem.base;
		rwops->hidden.mem.stop = rwops->hidden.mem.base + size;
		rwops->type = SDL_RWOPS_MEMORY;
	}

	return rwops;
}

SDL_RWops *SDL_RWFromFile(const char *file, const char *mode)
{
	if (!file || !*file || !mode || !*mode)
	{
		SDL_SetError("Invalid arguments.");
		return NULL;
	}

	SDL_RWops *rwops = calloc(1, sizeof(SDL_RWops));
	if (rwops)
	{
		FILE *fp = fopen(file, mode);
		if (fp)
		{
			rwops->hidden.stdio.fp = fp;
			//rwops->hidden.stdio.autoclose = autoclose;
			rwops->type = SDL_RWOPS_STDFILE;
		}
		else
		{
			SDL_SetError("Couldn't open file.");
			free(rwops);
			rwops = NULL;
		}
	}

	return rwops;
}

int SDL_RWsize(struct SDL_RWops *context)
{//printf("%s:%d %d\n", __FUNCTION__, __LINE__, context->type);
	if (context->type == SDL_RWOPS_STDFILE)
	{
		FILE *fp = context->hidden.stdio.fp;
//printf("%s:%d %p\n", __FUNCTION__, __LINE__, fp);
		int pos = fseek(fp, 0, SEEK_CUR);//printf("%s:%d %d\n", __FUNCTION__, __LINE__, pos);
		if (pos < 0) {
			return -1;
		}
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);//printf("%s:%d %d\n", __FUNCTION__, __LINE__, size);

		fseek(fp, pos, SEEK_SET);

		return size;
	}

	return (int)(context->hidden.mem.stop - context->hidden.mem.base);
}

int SDL_RWseek(struct SDL_RWops *context, long offset, int whence)
{
	if (context->type == SDL_RWOPS_STDFILE)
	{
		FILE *fp = context->hidden.stdio.fp;
		fseek(fp, offset, whence);
		return ftell(fp);
	}

	Uint8 *newpos = 0; // shut up the compiler

	switch (whence) {
	case RW_SEEK_SET:
		newpos = context->hidden.mem.base + offset;
		break;
	case RW_SEEK_CUR:
		newpos = context->hidden.mem.here + offset;
		break;
	case RW_SEEK_END:
		newpos = context->hidden.mem.stop + offset;
		break;
	}
	if (newpos < context->hidden.mem.base) {
		newpos = context->hidden.mem.base;
	}
	if (newpos > context->hidden.mem.stop) {
		newpos = context->hidden.mem.stop;
	}
	context->hidden.mem.here = newpos;
	return (int)(context->hidden.mem.here - context->hidden.mem.base);
}

int SDL_RWtell(struct SDL_RWops *context)
{
	//return SDL_RWseek(context, 0, RW_SEEK_CUR);
	if (context->type == SDL_RWOPS_STDFILE)
	{
		return ftell(context->hidden.stdio.fp);
	}

	return (int)(context->hidden.mem.here - context->hidden.mem.base);
}

size_t SDL_RWread(SDL_RWops *context, void *ptr, size_t size, size_t maxnum)
{
	if (context->type == SDL_RWOPS_STDFILE)
	{
		return fread(ptr, size, maxnum, context->hidden.stdio.fp);
	}

	size_t total_bytes = (maxnum * size);
	size_t mem_available = (context->hidden.mem.stop - context->hidden.mem.here);
	if (total_bytes > mem_available)
		total_bytes = mem_available;

	memcpy(ptr, context->hidden.mem.here, total_bytes);
	context->hidden.mem.here += total_bytes;

	return (total_bytes / size);
}


size_t SDL_RWwrite(struct SDL_RWops *context, const void *ptr, size_t size, size_t num)
{
	if (context->type == SDL_RWOPS_STDFILE)
	{
		return fwrite(ptr, size, num, context->hidden.stdio.fp);
	}

	if ((context->hidden.mem.here + (num * size)) > context->hidden.mem.stop) {
		num = (context->hidden.mem.stop - context->hidden.mem.here) / size;
	}
	memcpy(context->hidden.mem.here, ptr, num * size);
	context->hidden.mem.here += num * size;
	return num;
}

int SDL_RWclose(struct SDL_RWops *context)
{
	if (context->type == SDL_RWOPS_STDFILE)
	{
		fclose(context->hidden.stdio.fp);
	}

	free(context);
	return 0;
}

Uint16 SDL_ReadLE16(SDL_RWops *src)
{
	Uint16 value = 0;
	SDL_RWread(src, &value, sizeof (value), 1);
	return SDL_SwapLE16(value);
}

Uint32 SDL_ReadLE32(SDL_RWops *src)
{
	Uint32 value = 0;
	SDL_RWread(src, &value, sizeof (value), 1);
	return SDL_SwapLE32(value);
}
