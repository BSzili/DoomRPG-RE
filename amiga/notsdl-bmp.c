/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "SDL.h"

static SDL_Surface tmpSurface;
static SDL_PixelFormat tmpFormat;
static SDL_Palette tmpPalette;
static SDL_Color tmpColors[16]; // TODO reduce this

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	//printf("%s(%u, %d, %d, %d, %u, %u, %u, %u)\n", __FUNCTION__, flags, width, height, depth, Rmask, Gmask, Bmask, Amask);
	/*
	SDL_Surface *surface = calloc(sizeof(SDL_Surface) + sizeof(SDL_PixelFormat) + sizeof(SDL_Palette), 1);
	if (surface) {
		surface->w = width;
		surface->h = height;
		surface->pitch = width;
		surface->format = (SDL_PixelFormat *)(surface + 1);
		surface->format->palette = (SDL_Palette *)(surface->format + 1);
		printf("surface %p format %p palette %p", surface, surface->format, surface->format->palette);
		surface->format->palette->ncolors = 16;
	}
	*/
	SDL_Surface *surface = &tmpSurface;
	surface->colorkey = (Uint32)-1; // invalid default value
	surface->w = width;
	surface->h = height;
	surface->pitch = width;
	surface->format = &tmpFormat;
	surface->format->palette = &tmpPalette;
	//surface->format->palette->ncolors = 16;
	surface->format->palette->colors = tmpColors;
	surface->pixels = malloc(surface->h*surface->pitch);
	if (!surface->pixels) {
		return NULL;
	}

	return surface;
}

void SDL_FreeSurface(SDL_Surface *surface)
{
	if (surface->pixels) {
		free(surface->pixels);
		surface->pixels = NULL;
	}
}

int SDL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key)
{
	//printf("%s(%p, %d, %u)\n", __FUNCTION__, surface, flag, key);
	surface->colorkey = key;
	return 0;
}

Uint32 SDL_MapRGB(const SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Palette *palette = format->palette;
	//printf("%s(%p, %u, %u, %u)\n", __FUNCTION__, format, r, g, b);
	Uint32 key = 0;
	for (int i = 0; i < palette->ncolors; ++i) {
		if (palette->colors[i].b == b && palette->colors[i].g == g && palette->colors[i].r == r) {
			key = i;
			break;
		}
	}
	return key;

}

/*
static void SaveXPM2(const char *name, SDL_Surface *surface)
{
	FILE *fp = fopen(name, "w");
	if (!fp)
		return;

	SDL_Palette *palette = surface->format->palette;

	fprintf(fp, "! XPM2\n");
	fprintf(fp, "%d %d %d %d\n", surface->w, surface->h, palette->ncolors, 1);
	for (int i = 0; i < palette->ncolors; ++i) {
		fprintf(fp, "%c c #%02X%02X%02X\n", 'a' + i, palette->colors[i].r, palette->colors[i].g, palette->colors[i].b);
	}
	unsigned char *pixels = (unsigned char *)surface->pixels;
	for (int i = 0; i < surface->h; i++)
	{
		for (int j = 0; j < surface->w; j++)
		{
			int c = *pixels++;
			fputc('a' + c, fp);
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
}
*/

SDL_Surface *SDL_LoadBMP_RW(SDL_RWops *src, int freesrc)
{
    SDL_bool was_error;
    int fp_offset = 0;
    int bmpPitch;
    int i, pad;
    SDL_Surface *surface;
    SDL_Palette *palette;
    Uint8 *bits;
    Uint8 *top, *end;

    /* The Win32 BMP file header (14 bytes) */
    char magic[2];
    /* Uint32 bfSize = 0; */
    /* Uint16 bfReserved1 = 0; */
    /* Uint16 bfReserved2 = 0; */
    Uint32 bfOffBits = 0;

    /* The Win32 BITMAPINFOHEADER struct (40 bytes) */
    Uint32 biSize = 0;
    Sint32 biWidth = 0;
    Sint32 biHeight = 0;
    /* Uint16 biPlanes = 0; */
    Uint16 biBitCount = 0;
    Uint32 biCompression = 0;
    /* Uint32 biSizeImage = 0; */
    /* Sint32 biXPelsPerMeter = 0; */
    /* Sint32 biYPelsPerMeter = 0; */
    Uint32 biClrUsed = 0;
    /* Uint32 biClrImportant = 0; */

    /* Make sure we are passed a valid data source */
    surface = NULL;
    was_error = SDL_FALSE;
    if (src == NULL) {
        was_error = SDL_TRUE;
        goto done;
    }

    /* Read in the BMP file header */
    fp_offset = SDL_RWtell(src);
    SDL_ClearError();
    if (SDL_RWread(src, magic, 1, 2) != 2) {
        //SDL_Error(SDL_EFREAD);
        was_error = SDL_TRUE;
        goto done;
    }
    if (SDL_strncmp(magic, "BM", 2) != 0) {
        SDL_SetError("File is not a Windows BMP file");
        was_error = SDL_TRUE;
        goto done;
    }
    /* bfSize = */ SDL_ReadLE32(src);
    /* bfReserved1 = */ SDL_ReadLE16(src);
    /* bfReserved2 = */ SDL_ReadLE16(src);
    bfOffBits = SDL_ReadLE32(src);

    /* Read the Win32 BITMAPINFOHEADER */
    biSize = SDL_ReadLE32(src);//printf("biSize %d\n", biSize);
	if (biSize != 40) {
		SDL_SetError("BITMAPINFOHEADER is not the correct size");
		was_error = SDL_TRUE;
		goto done;
	}
    
	//Uint32 headerSize;
	biWidth = SDL_ReadLE32(src);//printf("biWidth %d\n", biWidth);
	biHeight = SDL_ReadLE32(src);//printf("biHeight %d\n", biHeight);
	/* biPlanes = */ SDL_ReadLE16(src);
	biBitCount = SDL_ReadLE16(src);//printf("biBitCount %d\n", biBitCount);
	if (biBitCount != 4) {
		SDL_SetError("Only 16 color bitmaps are supported");
		was_error = SDL_TRUE;
		goto done;
	}
	biCompression = SDL_ReadLE32(src);//printf("biCompression %d\n", biCompression);
	if (biCompression != 0) {
		SDL_SetError("Compressed BMP files are not supported");
		was_error = SDL_TRUE;
		goto done;
	}
	/* biSizeImage = */ SDL_ReadLE32(src);
	/* biXPelsPerMeter = */ SDL_ReadLE32(src);
	/* biYPelsPerMeter = */ SDL_ReadLE32(src);
	biClrUsed = SDL_ReadLE32(src);
	/* biClrImportant = */ SDL_ReadLE32(src);

    if (biHeight < 0) {
		SDL_SetError("Top-down BMP files are not supported");
		was_error = SDL_TRUE;
		goto done;
    }

    /* Check for read error */
    if (SDL_strcmp(SDL_GetError(), "") != 0) {
        was_error = SDL_TRUE;
        goto done;
    }

    /* Create a compatible surface, note that the colors are RGB ordered */
    surface = SDL_CreateRGBSurface(0, biWidth, biHeight, 8, 0, 0, 0, 0);
    if (surface == NULL) {
        was_error = SDL_TRUE;
        goto done;
    }

    /* Load the palette, if any */
    palette = surface->format->palette;
    if (palette) {
        if (biClrUsed == 0) {
            biClrUsed = 16;
        }//printf("biClrUsed %d\n", biClrUsed);
		palette->ncolors = biClrUsed;
		for (i = 0; i < (int) biClrUsed; ++i) {
			SDL_RWread(src, &palette->colors[i].b, 1, 1);
			SDL_RWread(src, &palette->colors[i].g, 1, 1);
			SDL_RWread(src, &palette->colors[i].r, 1, 1);
			SDL_RWread(src, &palette->colors[i].a, 1, 1);
//printf("- %3d (%03d, %03d, %03d)\n", i,palette->colors[i].b, palette->colors[i].g, palette->colors[i].r);
		}
    }

    /* Read the surface pixels.  Note that the bmp image is upside down */
    if (SDL_RWseek(src, fp_offset + bfOffBits, RW_SEEK_SET) < 0) {
        //SDL_Error(SDL_EFSEEK);
        was_error = SDL_TRUE;
        goto done;
    }
    top = (Uint8 *)surface->pixels;
    end = (Uint8 *)surface->pixels+(surface->h*surface->pitch);
    bmpPitch = (biWidth + 1) >> 1;
    pad = (((bmpPitch) % 4) ? (4 - ((bmpPitch) % 4)) : 0);
    bits = end - surface->pitch;
//printf("bmpPitch %d pad %d\n", bmpPitch, pad);
	// TODO simplify this loop for the 4bpp case
    while (bits >= top && bits < end) {
		Uint8 pixel = 0;
		for (i = 0; i < surface->w; ++i) {
			if (i % (8 / 4) == 0) {
				if (!SDL_RWread(src, &pixel, 1, 1)) {
					SDL_SetError("Error reading from BMP");
					was_error = SDL_TRUE;
					goto done;
				}
			}
			*(bits + i) = (pixel >> (8 - 4));
			pixel <<= 4;
		}

        /* Skip padding bytes, ugh */
        if (pad) {
            Uint8 padbyte;
            for (i = 0; i < pad; ++i) {
                SDL_RWread(src, &padbyte, 1, 1);
            }
        }
        bits -= surface->pitch;
    }
		//SaveXPM2("dump.xpm", surface);
  done:
    if (was_error) {
        if (src) {
            SDL_RWseek(src, fp_offset, RW_SEEK_SET);
        }
        SDL_FreeSurface(surface);
        surface = NULL;
    }
    if (freesrc && src) {
        SDL_RWclose(src);
    }
    return (surface);
}
