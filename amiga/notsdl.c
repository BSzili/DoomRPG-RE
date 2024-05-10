#include "SDL.h"

#include <proto/lowlevel.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
//#include <proto/layers.h>
#include <proto/timer.h>
#include <proto/icon.h>
#include <clib/alib_protos.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuitionbase.h>
#include <workbench/startup.h>

#include <libraries/Picasso96.h>
#include <proto/Picasso96.h>
struct Library *P96Base;

#include <SDI_compiler.h>

#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))
#define ABS(_x) ((_x) < 0 ? -(_x) : (_x))
#define MAKE_RGB565(red, green, blue) (((red) >> 3) << 11) | (((green) >> 2) << 5) | ((blue) >> 3)
//#define PRESENT_BENCHMARK
#define TEXTURE_DIRECT
#define RENDERER_DIRECT
#define LL_CREATEKEYS

struct Library *LowLevelBase;

struct SDL_Window
{
	struct Window *window;
	int w, h;
};

struct SDL_Texture
{
	Uint16 *pixels;
	int w, h;
	SDL_bool masked;
	Uint16 colorkey;
#ifdef TEXTURE_DIRECT
	SDL_bool allocated;
#endif
};

struct SDL_Renderer
{
	struct Window *window;
	//struct RastPort *rp;
	Uint16 *pixels;
	SDL_Point clip1;
	SDL_Point clip2;
	SDL_bool clipped;
	int w, h;
	int pitch;
#ifdef RENDERER_DIRECT
	SDL_bool allocated;
#endif
};


Uint8 keystate[SDL_NUM_SCANCODES];
#ifdef LL_CREATEKEYS
static Uint8 joystate[SDL_CONTROLLER_BUTTON_MAX];
#else
static ULONG portstate;
#endif

static struct Screen *screen;
static struct Window *window;
static ULONG fsMonitorID = INVALID_ID;
static char wndPubScreen[32] = {"Workbench"};
static UWORD *pointermem;
static int use_ham;
static SDL_Window sdlWindow;
static SDL_Color renderColor;
//static LONG renderPen = -1;
static Uint16 renderColor565;
static SDL_Renderer sdlRenderer;

char __stdiowin[]="CON:////Doom RPG Console/CLOSE/WAIT";
extern int __argc;
extern char **__argv;


extern void c2p_2rgb565_4rgb555h8_040_init(REG(d0, LONG chunkyx), REG(d1, LONG chunkyy), REG(d2, LONG scroffsx), REG(d3, LONG scroffsy), REG(d4, LONG rowlen), REG(d5, LONG bplsize), REG(d6, LONG chunkylen));
extern void c2p_2rgb565_3rgb555h8_040_init(REG(d0, LONG chunkyx), REG(d1, LONG chunkyy), REG(d2, LONG scroffsx), REG(d3, LONG scroffsy), REG(d4, LONG rowlen), REG(d5, LONG bplsize), REG(d6, LONG chunkylen));
extern void c2p_2rgb565_3rgb555h8_040(REG(a0, APTR c2pscreen), REG(a1, APTR bitplanes));
extern void c2p_2rgb565_4rgb555h8_040(REG(a0, APTR c2pscreen), REG(a1, APTR bitplanes));


int SDL_RenderSetClipRect(SDL_Renderer *renderer, const SDL_Rect *rect)
{
	if (rect != NULL) {
		//printf("%s(%p,%p) (%d,%d,%d,%d)\n", __FUNCTION__, renderer, rect, rect->x, rect->y, rect->w, rect->h);
		renderer->clip1.x = rect->x;
		renderer->clip2.x = rect->x + rect->w - 1;
		renderer->clip1.y = rect->y;
		renderer->clip2.y = rect->y + rect->h - 1;
		renderer->clipped = SDL_TRUE;
	} else {
		renderer->clipped = SDL_FALSE;
	}

	return 0;
}

#ifdef PRESENT_BENCHMARK
static int UpdateBenchmark(SDL_Renderer *renderer, ULONG secs)
{
	ULONG timer_sig;
	struct timerequest *timerio;
	struct MsgPort *timerport;
	ULONG frames = 0;
	ULONG fps = 0;

	if ((timerport = CreateMsgPort()))
	{
		if ((timerio = (struct timerequest *)CreateIORequest(timerport, sizeof(struct timerequest))))
		{
			if (!OpenDevice((STRPTR)TIMERNAME, UNIT_MICROHZ, (struct IORequest *)timerio, 0))
			{
				timer_sig = 1L << timerport->mp_SigBit;

				timerio->tr_node.io_Command = TR_ADDREQUEST;
				timerio->tr_time.tv_secs = secs;
				timerio->tr_time.tv_micro = 0;
				BeginIO((struct IORequest *)timerio);

				do
				{
					SDL_RenderPresent(renderer);
					frames++;
				} while (!(SetSignal(0, timer_sig) & timer_sig));

				while (GetMsg(timerport)) ; // consume the timer messages

				fps = frames/secs;

				printf("frames: %lu secs: %lu fps: %lu\n", frames, secs, fps);
			}
			DeleteIORequest((struct IORequest *)timerio);
		}
		DeleteMsgPort(timerport);
	}

	return fps;
}
#endif

void SDL_DestroyRenderer(SDL_Renderer *renderer)
{
	if (renderer->pixels) {
#ifdef RENDERER_DIRECT
		if (renderer->allocated) {
#endif
			free(renderer->pixels);
#ifdef RENDERER_DIRECT
		}
#endif
		renderer->pixels = NULL;
	}
}

SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags)
{
	SDL_Renderer *renderer = &sdlRenderer;

	renderer->w = window->w;
	renderer->h = window->h;
	renderer->pitch = window->w * 2;
	renderer->pixels = calloc(renderer->pitch * renderer->h, 1);
	if (!renderer->pixels) {
		SDL_SetError("Can't allocate the renderer buffer");
		return NULL;
	}
#ifdef RENDERER_DIRECT
	renderer->allocated = SDL_TRUE;
#endif
	renderer->window = window->window;
#ifdef PRESENT_BENCHMARK
	UpdateBenchmark(renderer, 1);
#endif

	return renderer;
}

void SDL_RenderPresent(SDL_Renderer *renderer)
{
	struct Window *win = renderer->window;

	//printf("%s\n", __FUNCTION__);

	if (use_ham) {
		//struct BitMap *bm = window->WScreen->RastPort.BitMap;
		struct BitMap *bm = window->RPort->BitMap;
#if 0
		static char ham_buffer[1280*240*6];
		c2p_2rgb565_4rgb555h8_040(renderer->pixels, ham_buffer);
		memcpy(bm->Planes[0], ham_buffer, bm->BytesPerRow * bm->Rows * 6);
#else
		//WaitBOVP(&win->WScreen->ViewPort);
		if (use_ham == 4) {
			c2p_2rgb565_4rgb555h8_040(renderer->pixels, bm->Planes[1]);
		} else {
			c2p_2rgb565_3rgb555h8_040(renderer->pixels, bm->Planes[1]);
		}
#endif
	} else /*if (P96Base)*/ {
		struct RenderInfo ri;
		ri.Memory = renderer->pixels;
		ri.BytesPerRow = renderer->pitch;
		ri.RGBFormat = RGBFB_R5G6B5;
		p96WritePixelArray(&ri, 0, 0, win->RPort, win->BorderLeft, win->BorderTop, renderer->w, renderer->h);
	}
}

int SDL_RenderClear(SDL_Renderer *renderer)
{
	SDL_Rect rect;

	rect.x = 0;
	rect.y = 0;
	rect.w = renderer->w;
	rect.h = renderer->h;
	SDL_RenderFillRect(renderer, &rect);

	return 0;
}

int SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect)
{
	//printf("%s %p (%d,%d,%d,%d) -> (%d,%d,%d,%d)\n", __FUNCTION__, texture, srcrect->x, srcrect->y, srcrect->w,  srcrect->h, dstrect->x, dstrect->y, dstrect->w, dstrect->h);

#ifdef RENDERER_DIRECT
	if (renderer->pixels == texture->pixels) {
		//printf("%s self update\n", __FUNCTION__);
		return 0;
	}
#endif

	WORD xSize = dstrect->w;
	WORD ySize = dstrect->h;
	if (srcrect->w < dstrect->w) xSize = srcrect->w;
	if (srcrect->h < dstrect->h) ySize = srcrect->h;

	WORD xSrc = srcrect->x;
	WORD ySrc = srcrect->y;
	WORD xDest = dstrect->x;
	WORD yDest = dstrect->y;

	if (renderer->clipped) {
		int cx1 = renderer->clip1.x;
		int cx2 = renderer->clip2.x;
		int cy1 = renderer->clip1.y;
		int cy2 = renderer->clip2.y;
		int dx1 = xDest;
		int dx2 = xDest + xSize - 1;
		int dy1 = yDest;
		int dy2 = yDest + ySize - 1;
		if (dx1 > cx2 || dy1 > cy2 || dx2 < cx1 || dy2 < cy1) {
			return 0; // completely clipped
		}
		int diff = cx1 - dx1;
		if (diff > 0) {
			xDest += diff;
			xSrc += diff;
			xSize -= diff;
		}
		diff = cy1 - dy1;
		if (diff > 0) {
			yDest += diff;
			ySrc += diff;
			ySize -= diff;
		}
		diff = dx2 - cx2;
		if (diff > 0) {
			xSize -= diff;
		}
		diff = dy2 - cy2;
		if (diff > 0) {
			ySize -= diff;
		}
	}

	Uint16 *srcpix = texture->pixels;
	int srcwidth = texture->w;

	Uint16 *dstpix = renderer->pixels;
	int dstwidth = renderer->w;

	if (!texture->masked) {
		if (xSrc == 0 && xDest == 0 && xSize == dstwidth) {	// blit the whole width of the screen
			int length = xSize * ySize;
			Uint16 *dst = dstpix + yDest * dstwidth;
			Uint16 *src = srcpix + ySrc * srcwidth;
			do {
				*dst++ = *src++;
			} while (--length);
		} else if (xSrc == 0 && xSize == srcwidth) {	// blit the whole width of the source
			WORD width = xSize;
			WORD height = ySize;
			int dstskip = dstwidth - width;
			Uint16 *dst = dstpix + (yDest * dstwidth) + xDest;
			Uint16 *src = srcpix + (ySrc * srcwidth);
			while (height--) {
				WORD n = width;
				do {
					*dst++ = *src++;
				} while (--n);
				dst += dstskip;
			}
		} else {	// general case
			WORD width = xSize;
			WORD height = ySize;
			int dstskip = dstwidth - width;
			int srcskip = srcwidth - width;
			Uint16 *dst = dstpix + (yDest * dstwidth) + xDest;
			Uint16 *src = srcpix + (ySrc * srcwidth) + xSrc;
			while (height--) {
				WORD n = width;
				do {
					*dst++ = *src++;
				} while (--n);
				dst += dstskip;
				src += srcskip;
			}
		}
	} else {
		Uint16 colorkey = texture->colorkey;
		if (xSrc == 0 && xSize == srcwidth) {	// blit the whole width of the source
			WORD width = xSize;
			WORD height = ySize;
			int dstskip = dstwidth - width;
			Uint16 *dst = dstpix + (yDest * dstwidth) + xDest;
			Uint16 *src = srcpix + (ySrc * srcwidth);
			while (height--) {
				WORD n = width;
				do {
					Uint16 col = *src++;
					if (col != colorkey) {
						*dst = col;
					}
					dst++;
				} while (--n);
				dst += dstskip;
			}
		} else {
			WORD width = xSize;
			WORD height = ySize;
			int dstskip = dstwidth - width;
			int srcskip = srcwidth - width;
			Uint16 *dst = dstpix + (yDest * dstwidth) + xDest;
			Uint16 *src = srcpix + (ySrc * srcwidth) + xSrc;
			while (height--) {
				WORD n = width;
				do {
					Uint16 col = *src++;
					if (col != colorkey) {
						*dst = col;
					}
					dst++;
				} while (--n);
				dst += dstskip;
				src += srcskip;
			}
		}
	}

	return 0;
}

int SDL_SetRenderDrawColor(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if (renderColor.r != r || renderColor.g != g || renderColor.b != b) {
		renderColor.r = r;
		renderColor.g = g;
		renderColor.b = b;

		renderColor565 = MAKE_RGB565(r, g, b);
	}

	return 0;
}

int SDL_RenderFillRect(SDL_Renderer *renderer, const SDL_Rect *rect)
{
	// the menu has a buggy rect that can cause problems in low resolutions
	WORD xSize = rect->w;
	WORD ySize = rect->h;
	WORD xDest = rect->x;
	WORD yDest = rect->y;
	int cx2 = renderer->w;
	int cy2 = renderer->h;
	int dx2 = xDest + xSize;
	int dy2 = yDest + ySize;
	int diff;
	diff = dx2 - cx2;
	if (diff > 0) {
		xSize -= diff;
	}
	diff = dy2 - cy2;
	if (diff > 0) {
		ySize -= diff;
	}

	/*
	if (rect->x + rect->w > renderer->w || rect->y + rect->h > renderer->h) {
		// TODO clipping
		printf("%s invalid rect (%d,%d,%d,%d)\n", __FUNCTION__, rect->x, rect->y, rect->w, rect->h);
		return -1;
	}
	*/

	Uint16 *dstpix = renderer->pixels;
	int dstwidth = renderer->w;

	Uint16 color = renderColor565;
	if (rect->x == 0 && rect->w == dstwidth) {
		int length = xSize * ySize;
		Uint16 *pixel = dstpix + (rect->y * dstwidth);
		while (length--) {
			*pixel++ = color;
		}
	} else {
		int width = xSize;
		int height = ySize;
		int pitch = dstwidth;
		int skip = pitch - width;
		Uint16 *pixel = dstpix + rect->y * pitch + rect->x;
		while (height--) {
			int n = width;
			while (n--) {
				*pixel++ = color;
			}
			pixel += skip;
		}
	}

	return 0;
}

int SDL_RenderDrawRect(SDL_Renderer *renderer, const SDL_Rect *rect)
{
	// TODO inline
	SDL_RenderDrawLine(renderer, rect->x, rect->y, rect->x+rect->w-1, rect->y);
	SDL_RenderDrawLine(renderer, rect->x, rect->y+rect->h-1, rect->x+rect->w-1, rect->y+rect->h-1);
	SDL_RenderDrawLine(renderer, rect->x, rect->y+1, rect->x, rect->y+rect->h-2);
	SDL_RenderDrawLine(renderer, rect->x+rect->w-1, rect->y+1, rect->x+rect->w-1, rect->y+rect->h-2);

	return 0;
}


int SDL_RenderDrawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2)
{
	SDL_bool clipped = renderer->clipped;
	if (clipped) {
		int cx1 = renderer->clip1.x;
		int cx2 = renderer->clip2.x;
		int cy1 = renderer->clip1.y;
		int cy2 = renderer->clip2.x;
		if (x1 > cx2 || y1 > cy2 || x2 < cx1 || y2 < cy1) {
			return 0; // completely clipped
		}
	}

	Uint16 *dstpix = renderer->pixels;
	int dstwidth = renderer->w;

	Uint16 color = renderColor565;
	WORD length;
	Uint16 *pixel;
	if (y1 == y2) {	// horizontal
		if (clipped) {
			int cx1 = renderer->clip1.x;
			int cx2 = renderer->clip2.x;
			if (x1 < cx1) x1 = cx1;
			if (x2 > cx2) x2 = cx2;
		}
		if (x1 <= x2) {
			pixel = dstpix + (y1 * dstwidth) + x1;
			length = x2-x1+1;
		} else {
			pixel = dstpix + (y1 * dstwidth) + x2;
			length = x1-x2+1;
		}
		do {
			*pixel++ = color;
		} while (--length);
	} else if (x1 == x2) {	// vertical
		if (clipped) {
			int cy1 = renderer->clip1.y;
			int cy2 = renderer->clip2.y;
			if (y1 < cy1) y1 = cy1;
			if (y2 > cy2) y2 = cy2;
		}
		if (y1 <= y2) {
			pixel = dstpix + (y1 * dstwidth) + x1;
			length = y2-y1+1;
		} else {
			pixel = dstpix + (y2 * dstwidth) + x1;
			length = y1-y2+1;
		}
		do {
			*pixel = color;
			pixel += dstwidth;
		} while (--length);
	} else {	// diagonal
		if (clipped) {
			int cx1 = renderer->clip1.x;
			int cx2 = renderer->clip2.x;
			int cy1 = renderer->clip1.y;
			int cy2 = renderer->clip2.y;
			if (x1 < cx1) x1 = cx1;
			if (y1 < cy1) y1 = cy1;
			if (x2 > cx2) x2 = cx2;
			if (y2 > cy2) y2 = cy2;
		}
		if (y1 <= y2) {
			pixel = dstpix + (y1 * dstwidth) + x1;
			if (x1 <= x2) {
				++dstwidth;
			} else {
				--dstwidth;
			}
			length = (y2-y1+1);
		} else {
			pixel = dstpix + (y2 * dstwidth) + x2;
			if (x2 <= x1) {
				++dstwidth;
			} else {
				--dstwidth;
			}
			length = (y1-y2+1);
		}
		do {
			*pixel = color;
			pixel += dstwidth;
		} while (--length);
	}
	/* UNUSED general case
	int dx, dy, err, sx, sy, e2;
	dx = ABS(x2 - x1);
	dy = ABS(y2 - y1);
	if (x1 < x2) sx = 1; else sx = -1;
	if (y1 < y2) sy = 1; else sy = -1;
	err = dx-dy;

	while (TRUE) {
		WriteRGBPixel(rp, x1, y1, color);
		if ((x1 == x2) && (y1 == y2)) break;
		e2 = 2*err;
		if (e2 > -dy) {
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y1 += sy;
		}
	}
	*/

	return 0;
}

int SDL_RenderDrawPoints(SDL_Renderer *renderer, const SDL_Point *points, int count)
{
	// not implemented, only used by SDL_RenderDrawCircle / DoomRPG_drawCircle
	return 1;
}

int SDL_SetRenderDrawBlendMode(SDL_Renderer *renderer, SDL_BlendMode blendMode)
{
	// not implemented, used by the bind menu
	return -1;
}


void SDL_DestroyTexture(SDL_Texture *texture)
{
	if (texture) {
		if (texture->pixels) {
#ifdef TEXTURE_DIRECT
			if (texture->allocated) {
				texture->allocated = SDL_FALSE;
#endif
				free(texture->pixels);
				texture->pixels = NULL;
#ifdef TEXTURE_DIRECT
			}
#endif
		}
		free(texture);
	}
}

SDL_Texture *SDL_CreateTexture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h)
{
	//printf("%s(%p,%u,%d,%d,%d)\n", __FUNCTION__, renderer, format, access, w, h);

	struct SDL_Texture *texture = calloc(sizeof(SDL_Texture), 1);
	if (texture) {
		texture->w = w;
		texture->h = h;
#ifndef TEXTURE_DIRECT
		int pitch = w * 2;
		texture->pixels = calloc(pitch * h, 1);
		if (!texture->pixels) {
			SDL_SetError("Can't allocate the texture buffer");
			return NULL;
		}
#endif
	} else {
		SDL_SetError("Can't allocate the texture structure");
	}

	return texture;
}

int SDL_UpdateTexture(SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch)
{
#ifdef TEXTURE_DIRECT
/*
	if (texture->allocated) {
		texture->allocated = SDL_FALSE;
		if (texture->pixels) {
			free(texture->pixels);
			texture->pixels = NULL;
		}
	}
	*/
	if (!texture->pixels) {
		//printf("%s texture %p pixels %p\n", __FUNCTION__, texture, pixels);
		texture->allocated = SDL_FALSE;
		texture->pixels = pixels;
#ifdef RENDERER_DIRECT
		SDL_Renderer *renderer = &sdlRenderer;
		if (renderer->allocated) {
			renderer->allocated = SDL_FALSE;
			if (renderer->pixels) {
				//printf("%s freeing the renderer temp bitmap\n", __FUNCTION__);
				free(renderer->pixels);
				renderer->pixels = NULL;
			}
			//printf("%s renderer %p pixels %p\n", __FUNCTION__, renderer, pixels);
			renderer->pixels = pixels;
		}
#endif
	}
#else
	const Uint16 *srcpix = pixels;
	int srcwidth = pitch >> 1;
	Uint16 *dstpix = texture->pixels;
	int dstwidth = texture->w;
	int dstheight = texture->h;

	if (srcwidth == dstwidth) {
		int length = dstwidth * dstheight;
		Uint16 *dst = dstpix;
		const Uint16 *src = srcpix;
		while (length--) {
			*dst++ = *src++;
		}
	} else {
		for (int j = 0; j < dstheight; j++) {
			const Uint16 *s = srcpix;
			Uint16 *d = dstpix;
			for (int i = 0; i < dstwidth; i++) {
				*d++ = *s++;
			}
			srcpix += srcwidth;
			dstpix += dstwidth;
		}
	}
#endif

	return 0;
}

int SDL_SetTextureColorMod(SDL_Texture *texture, Uint8 r, Uint8 g, Uint8 b)
{
	// not implemented, only used to color the menu fonts
	return -1;
}

SDL_Texture *SDL_CreateTextureFromSurface(SDL_Renderer *renderer, SDL_Surface *surface)
{
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, 0, surface->w, surface->h);
	if (texture) {
#ifdef TEXTURE_DIRECT
		int h = texture->h;
		int pitch = texture->w * 2;
		if ((texture->pixels = calloc(pitch * h, 1))) {
			texture->allocated = SDL_TRUE;
#endif

			SDL_Palette *palette = surface->format->palette;
			SDL_Color *colors = palette->colors;

			const Uint8 *srcpix = surface->pixels;
			//int srcpitch = surface->pitch;
			Uint16 *dstpix = texture->pixels;
			Sint16 dstwidth = texture->w;
			Sint16 dstheight = texture->h;

			const Uint8 *src = srcpix;
			Uint16 *dst = dstpix;
			Sint16 height = dstheight;
			do {
				Sint16 width = dstwidth;
				do {
					Uint8 c = *src++;
					*dst++ = MAKE_RGB565(colors[c].r, colors[c].g, colors[c].b);
				} while (--width);
			} while (--height);

			//printf("%s colorkey %d ncolors %d\n", __FUNCTION__, surface->colorkey, palette->ncolors);
			if (surface->colorkey < palette->ncolors) {
				const Uint8 *src = surface->pixels;
				Uint8 key = surface->colorkey;
				Sint16 height = surface->h;
				do {
					Sint16 width = surface->w;
					do {
						Uint8 c = *src++;
						if (c == key) {
							texture->masked = SDL_TRUE;
							texture->colorkey = MAKE_RGB565(palette->colors[c].r, palette->colors[c].g, palette->colors[c].b);
							break;
						}
					} while (--width);
				} while (--height && !texture->masked);
			}
#ifdef TEXTURE_DIRECT
		} else {
			SDL_SetError("Can't allocate the texture buffer");
		}
#endif
	}
	return texture;
}



// various

extern int SDL_TimerInit(void);
//extern void SDL_TimerQuit(void);

static void parseTooltypes(void) {
	char *exename;
	struct DiskObject *appicon;

	if (__argc == 0) {
		struct WBStartup *startup = (struct WBStartup *)__argv;
		exename = (char *)startup->sm_ArgList->wa_Name;
	} else {
		exename = __argv[0];
	}

	if ((appicon = GetDiskObject((STRPTR)exename))) {
		char *value;

		if ((value = (char *)FindToolType((CONST_STRPTR *)appicon->do_ToolTypes, (CONST_STRPTR)"FORCEMODE"))) {
			if (!strcmp(value, "NTSC"))
				fsMonitorID = NTSC_MONITOR_ID;
			else if (!strcmp(value, "PAL"))
				fsMonitorID = PAL_MONITOR_ID;
			else if (!strcmp(value, "MULTISCAN"))
				fsMonitorID = VGA_MONITOR_ID;
			else if (!strcmp(value, "EURO72"))
				fsMonitorID = EURO72_MONITOR_ID;
			else if (!strcmp(value, "EURO36"))
				fsMonitorID = EURO36_MONITOR_ID;
			else if (!strcmp(value, "SUPER72"))
				fsMonitorID = SUPER72_MONITOR_ID;
			else if (!strcmp(value, "DBLNTSC"))
				fsMonitorID = DBLNTSC_MONITOR_ID;
			else if (!strcmp(value, "DBLPAL"))
				fsMonitorID = DBLPAL_MONITOR_ID;
		}
		FreeDiskObject(appicon);
	}
}


int SDL_Init(Uint32 flags)
{
	//setbuf(stdout, NULL); // TODO remove
	parseTooltypes();
	P96Base = OpenLibrary((STRPTR)P96NAME, 2);
	if (P96Base) {
		ULONG modeID = INVALID_ID;

		modeID = p96BestModeIDTags(
			P96BIDTAG_Depth, 16,
			P96BIDTAG_NominalWidth, 320,
			P96BIDTAG_NominalHeight, 240,
			TAG_DONE);

		if (modeID == (ULONG)INVALID_ID) {
			modeID = p96BestModeIDTags(
				P96BIDTAG_Depth, 15,
				P96BIDTAG_NominalWidth, 320,
				P96BIDTAG_NominalHeight, 240,
				TAG_DONE);
		}

		if (modeID == (ULONG)INVALID_ID) {
			// TODO maybe check 32-bit as well before bailing
			CloseLibrary(P96Base);
			P96Base = NULL;
		}
	}
	/*
	if (!P96Base)
	{
		SDL_SetError("Can't open " P96NAME " v2.");
		return -1;
	}
	*/
	LowLevelBase = OpenLibrary("lowlevel.library", 0);
	if (LowLevelBase) {
#ifdef LL_CREATEKEYS
		SystemControl(SCON_AddCreateKeys, 1, TAG_DONE);
#endif
	}
	SDL_TimerInit();
	return 0;
}

void SDL_Quit(void)
{
	if (P96Base) {
		CloseLibrary(P96Base);
		P96Base = NULL;
	}
	if (LowLevelBase) {
#ifdef LL_CREATEKEYS
		SystemControl(SCON_RemCreateKeys, 1, TAG_DONE);
#endif
		CloseLibrary(LowLevelBase);
		LowLevelBase = NULL;
	}
	//SDL_TimerQuit();
}

// keyboard

static const char *keynames[] =
{
	"`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",							/*  10 */
	"-", "=", "\\", 0, "Insert", "Q", "W", "E", "R", "T",							/*  20 */
	"Y", "U", "I", "O", "P", "[", "]", 0, "End", "Keypad 2",						/*  30 */
	"Page Down", "A", "S", "D", "F", "G", "H", "J", "K", "L",							/*  40 */
	";", "\'", "#", 0, "Keypad 4", "Keypad 5", "Keypad 6", "<", "Z", "X",				/*  50 */
	"C", "V", "B", "N", "M", ",", ".", "/", 0, "Delete",							/*  60 */
	"Home", "Keypad 8", "Page Up", "Space", "Backspace", "Tab", "Keypad Enter", "Enter", "Escape", "Delete",	/*  70 */
	0, 0, 0, "Keypad -", 0, "Up", "Down", "Right", "Left", "F1",	/*  80 */
	"F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "Num Lock",						/*  90 */
	"Scroll Lock", "Keypad /", "Keypad *", "Keypad +", "Help", "Left Shift", "Right Shift", "Caps Lock", "Ctrl", "Left Alt",					/* 100 */
	"Right Alt", "Left Amiga", "Right Amiga", 0, 0, 0, 0, 0, 0, 0,									/* 110 */
};

static const unsigned char keyconv[] =
{
	SDL_SCANCODE_GRAVE, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9, SDL_SCANCODE_0,							/*  10 */
	SDL_SCANCODE_MINUS, SDL_SCANCODE_EQUALS, SDL_SCANCODE_BACKSLASH, 0/*U*/, SDL_SCANCODE_KP_0, SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R, SDL_SCANCODE_T,							/*  20 */
	SDL_SCANCODE_Y, SDL_SCANCODE_U, SDL_SCANCODE_I, SDL_SCANCODE_O, SDL_SCANCODE_P, SDL_SCANCODE_LEFTBRACKET, SDL_SCANCODE_RIGHTBRACKET, 0/*U*/, SDL_SCANCODE_KP_1 , SDL_SCANCODE_KP_2,						/*  30 */
	SDL_SCANCODE_KP_3, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F, SDL_SCANCODE_G, SDL_SCANCODE_H, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L,							/*  40 */
	SDL_SCANCODE_SEMICOLON, SDL_SCANCODE_APOSTROPHE, 0/*#*/, 0/*U*/, SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_5, SDL_SCANCODE_KP_6, 0/*<*/, SDL_SCANCODE_Z, SDL_SCANCODE_X,				/*  50 */
	SDL_SCANCODE_C, SDL_SCANCODE_V, SDL_SCANCODE_B, SDL_SCANCODE_N, SDL_SCANCODE_M, SDL_SCANCODE_COMMA, SDL_SCANCODE_PERIOD, SDL_SCANCODE_SLASH, 0/*U*/, SDL_SCANCODE_KP_PERIOD,							/*  60 */
	SDL_SCANCODE_KP_7, SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_9, SDL_SCANCODE_SPACE, SDL_SCANCODE_BACKSPACE, SDL_SCANCODE_TAB, SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_RETURN, SDL_SCANCODE_ESCAPE, SDL_SCANCODE_DELETE,	/*  70 */
	0/*U*/, 0/*U*/, 0/*U*/, SDL_SCANCODE_KP_MINUS, 0/*U*/, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, SDL_SCANCODE_F1,	/*  80 */
	SDL_SCANCODE_F2, SDL_SCANCODE_F3, SDL_SCANCODE_F4, SDL_SCANCODE_F5, SDL_SCANCODE_F6, SDL_SCANCODE_F7, SDL_SCANCODE_F8, SDL_SCANCODE_F9, SDL_SCANCODE_F10, SDL_SCANCODE_NUMLOCKCLEAR/*(*/,						/*  90 */
	SDL_SCANCODE_SCROLLLOCK/*)*/, SDL_SCANCODE_KP_DIVIDE, SDL_SCANCODE_KP_MULTIPLY, SDL_SCANCODE_KP_PLUS, SDL_SCANCODE_F12/*help*/, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_RSHIFT, SDL_SCANCODE_CAPSLOCK, SDL_SCANCODE_LCTRL, SDL_SCANCODE_LALT,					/* 100 */
	SDL_SCANCODE_RALT, SDL_SCANCODE_LGUI, SDL_SCANCODE_RGUI, 0, 0, 0, 0, 0, 0, 0,									/* 110 */
};

#define MAX_KEYCONV (sizeof keyconv / sizeof keyconv[0])

const char* SDL_GetScancodeName(SDL_Scancode scancode)
{
	int i, scan;
	// linear search is not the fastest, but this is only used for the key binding menu
	for (i=0; i < (int)MAX_KEYCONV; i++) {
		scan = keyconv[i];
		if (scan != 0 && scan == scancode) {
			return keynames[i];
		}
	}
	return "";
}

/*
const Uint8 *SDL_GetKeyboardState(int *numkeys)
{
	return keystate;
}
*/

#ifdef LL_CREATEKEYS

static int LowLevelToSDL[11] =
{
	SDL_CONTROLLER_BUTTON_B,				// RAWKEY_PORT1_BUTTON_BLUE
	SDL_CONTROLLER_BUTTON_START,			// RAWKEY_PORT1_BUTTON_PLAY
	SDL_CONTROLLER_BUTTON_LEFTSHOULDER,		// RAWKEY_PORT1_BUTTON_REVERSE
	SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,	// RAWKEY_PORT1_BUTTON_FORWARD
	SDL_CONTROLLER_BUTTON_X,				// RAWKEY_PORT1_BUTTON_GREEN
	SDL_CONTROLLER_BUTTON_Y,				// RAWKEY_PORT1_BUTTON_YELLOW
	SDL_CONTROLLER_BUTTON_A,				// RAWKEY_PORT1_BUTTON_RED
	SDL_CONTROLLER_BUTTON_DPAD_UP,			// RAWKEY_PORT1_JOY_UP
	SDL_CONTROLLER_BUTTON_DPAD_DOWN,		// RAWKEY_PORT1_JOY_DOWN
	SDL_CONTROLLER_BUTTON_DPAD_RIGHT,		// RAWKEY_PORT1_JOY_RIGHT
	SDL_CONTROLLER_BUTTON_DPAD_LEFT,		// RAWKEY_PORT1_JOY_LEFT
};

/*
#define RAWKEY_PORT1_BUTTON_BLUE	0x172
#define RAWKEY_PORT1_BUTTON_PLAY	0x173
#define RAWKEY_PORT1_BUTTON_REVERSE	0x174
#define RAWKEY_PORT1_BUTTON_FORWARD	0x175
#define RAWKEY_PORT1_BUTTON_GREEN	0x176
#define RAWKEY_PORT1_BUTTON_YELLOW	0x177
#define RAWKEY_PORT1_BUTTON_RED	0x178
#define RAWKEY_PORT1_JOY_UP		0x179
#define RAWKEY_PORT1_JOY_DOWN		0x17A
#define RAWKEY_PORT1_JOY_RIGHT		0x17B
#define RAWKEY_PORT1_JOY_LEFT		0x17C
*/

Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller, SDL_GameControllerButton button)
{
	return joystate[button];
}

#else

#define JPF_BUTTON_INVALID (1<<27) // bits 24 to 27 are unused
static ULONG SDLtoLowLevel[SDL_CONTROLLER_BUTTON_MAX] =
{
	JPF_BUTTON_RED,		// SDL_CONTROLLER_BUTTON_A
	JPF_BUTTON_BLUE,	// SDL_CONTROLLER_BUTTON_B
	JPF_BUTTON_GREEN,	// SDL_CONTROLLER_BUTTON_X
	JPF_BUTTON_YELLOW,	// SDL_CONTROLLER_BUTTON_Y
	JPF_BUTTON_INVALID,	// SDL_CONTROLLER_BUTTON_BACK
	JPF_BUTTON_INVALID,	// SDL_CONTROLLER_BUTTON_GUIDE
	JPF_BUTTON_PLAY,	// SDL_CONTROLLER_BUTTON_START
	JPF_BUTTON_INVALID,	// SDL_CONTROLLER_BUTTON_LEFTSTICK
	JPF_BUTTON_INVALID,	// SDL_CONTROLLER_BUTTON_RIGHTSTICK
	JPF_BUTTON_REVERSE,	// SDL_CONTROLLER_BUTTON_LEFTSHOULDER
	JPF_BUTTON_FORWARD,	// SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
	JPF_JOY_UP,			// SDL_CONTROLLER_BUTTON_DPAD_UP
	JPF_JOY_DOWN,		// SDL_CONTROLLER_BUTTON_DPAD_DOWN
	JPF_JOY_LEFT,		// SDL_CONTROLLER_BUTTON_DPAD_LEFT
	JPF_JOY_RIGHT,		// SDL_CONTROLLER_BUTTON_DPAD_RIGHT
};

Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller, SDL_GameControllerButton button)
{
	return (portstate & SDLtoLowLevel[button]) != 0;
}

#endif


int SDL_ShowMessageBox(const SDL_MessageBoxData *messageboxdata, int *buttonid)
{
	//printf("%s: %s\n", messageboxdata->title, messageboxdata->message);
	struct EasyStruct es;
	es.es_StructSize = sizeof(es);
	es.es_Flags = 0;
	es.es_Title = (STRPTR)messageboxdata->title;
	es.es_TextFormat = (STRPTR)messageboxdata->message;
	es.es_GadgetFormat = (STRPTR)messageboxdata->buttons->text;
	EasyRequestArgs(/*window*/NULL, &es, NULL, NULL);
	return 0;
}

int SDL_PollEvent(SDL_Event *event)
{
	struct IntuiMessage *imsg;
	int handled = 0;

	if (!window) {
		return 0;
	}

	if (screen && screen == IntuitionBase->FirstScreen && !(window->Flags & WFLG_WINDOWACTIVE)) {
		ActivateWindow(window);
	}

	/*while*/if ((imsg = (struct IntuiMessage *)GetMsg(window->UserPort))) {
		switch (imsg->Class)
		{
		case IDCMP_RAWKEY:
			{
				UWORD press = (imsg->Code & IECODE_UP_PREFIX) == 0;
				UWORD code = imsg->Code & ~IECODE_UP_PREFIX;
				UWORD qualifier = imsg->Qualifier;
				unsigned char scan;

				if (qualifier & IEQUALIFIER_REPEAT) {
					// we don't need key repeats
					break;
				}

				if (code < (int)MAX_KEYCONV) {
					scan = keyconv[code];
					keystate[scan] = press;
					//printf("%s code %x scan %d (%s) press %d\n", __FUNCTION__, code, scan, SDL_GetScancodeName(scan), press);
#ifdef LL_CREATEKEYS
				} else if (code >= RAWKEY_PORT1_BUTTON_BLUE && code <= RAWKEY_PORT1_JOY_LEFT) {
					scan = LowLevelToSDL[code - RAWKEY_PORT1_BUTTON_BLUE];
					joystate[scan] = press;
					//printf("%s code %x scan %d press %d\n", __FUNCTION__, code, scan, press);
#endif
				} else {
					//printf("%s %x %d\n", __FUNCTION__, code, press);
					break;
				}

				handled = 1;
			}
			break;
		case IDCMP_CLOSEWINDOW:
			event->type = SDL_WINDOWEVENT;
			event->window.event = SDL_WINDOWEVENT_CLOSE;
			handled = 1;
			break;
		}

		ReplyMsg((struct Message *)imsg);
	}

#ifndef LL_CREATEKEYS
	// update the joystick here
	if (LowLevelBase) {
		portstate = ReadJoyPort(1);
	}
#endif

	return handled;
}

int SDL_SetWindowFullscreen(SDL_Window *window, Uint32 flags)
{
	return -1; // not implemented
}

SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags)
{
	ULONG modeID = INVALID_ID;

	if (window) {
		SDL_SetError("The window is already open");
		return NULL;
	}

	if (!(flags & SDL_WINDOW_FULLSCREEN)) {
		int maxx=0, maxy=0;
		int bpp = 0;
		if ((screen = LockPubScreen((STRPTR)wndPubScreen))) {
			bpp = GetBitMapAttr(screen->RastPort.BitMap, BMA_DEPTH);
			maxx = screen->Width;
			maxy = screen->Height;
			UnlockPubScreen(NULL, screen);
			screen = NULL;
		}
		if (bpp < 15 || maxx < w || maxy < h) {
			// the public screen is not suitable, switch to fullscreen
			flags |= SDL_WINDOW_FULLSCREEN;
		}
	}

	use_ham = 0;

	if (flags & SDL_WINDOW_FULLSCREEN) {
		if (P96Base && fsMonitorID == (ULONG)INVALID_ID) {
			modeID = p96BestModeIDTags(
				P96BIDTAG_Depth, 16,
				P96BIDTAG_NominalWidth, w,
				P96BIDTAG_NominalHeight, h,
				TAG_DONE);

			if (modeID == (ULONG)INVALID_ID) {
				modeID = p96BestModeIDTags(
					P96BIDTAG_Depth, 15,
					P96BIDTAG_NominalWidth, w,
					P96BIDTAG_NominalHeight, h,
					TAG_DONE);
			}
		}

		if (modeID == (ULONG)INVALID_ID) {
			struct DimensionInfo diminfo;
			struct DisplayInfo dispinfo;
			ULONG result;
			int width, height;

#if 0
			modeID = INVALID_ID;
			while ((modeID = NextDisplayInfo(modeID)) != (ULONG)INVALID_ID)
			{
				if (!GetDisplayInfoData(NULL, (UBYTE *)&diminfo, sizeof(diminfo), DTAG_DIMS, modeID))
					continue;

				if (!GetDisplayInfoData(NULL, (UBYTE *)&dispinfo, sizeof(dispinfo), DTAG_DISP, modeID))
					continue;

				int width = diminfo.Nominal.MaxX + 1;
				int height = diminfo.Nominal.MaxY + 1;

				printf("id %08lx width %d height %d depth %d HAM %ld\n", modeID, width, height, diminfo.MaxDepth, (dispinfo.PropertyFlags & DIPF_IS_HAM));
			}
#endif

			modeID = BestModeID(
				BIDTAG_NominalWidth, w*4,
				BIDTAG_NominalHeight, h,
				BIDTAG_Depth, 8,
				BIDTAG_DIPFMustHave, DIPF_IS_HAM,
				(fsMonitorID == (ULONG)INVALID_ID) ? TAG_IGNORE : BIDTAG_MonitorID, fsMonitorID,
				TAG_DONE);

			result = GetDisplayInfoData(NULL, (UBYTE *)&diminfo, sizeof(diminfo), DTAG_DIMS, modeID);
			width = diminfo.Nominal.MaxX + 1;
			height = diminfo.Nominal.MaxY + 1;
			//printf("%s width %d height %d\n", __FUNCTION__, width, height);

			if (modeID != (ULONG)INVALID_ID && result && width >= w*4 && height >= h /*&& width < 1280*/) {
				use_ham = 4;
			} else {
				modeID = BestModeID(
					BIDTAG_NominalWidth, w*3,
					BIDTAG_NominalHeight, h,
					BIDTAG_Depth, 8,
					BIDTAG_DIPFMustHave, DIPF_IS_HAM,
					(fsMonitorID == (ULONG)INVALID_ID) ? TAG_IGNORE : BIDTAG_MonitorID, fsMonitorID,
					TAG_DONE);

				result = GetDisplayInfoData(NULL, (UBYTE *)&diminfo, sizeof(diminfo), DTAG_DIMS, modeID);
				width = diminfo.Nominal.MaxX + 1;
				height = diminfo.Nominal.MaxY + 1;
				//printf("%s width %d height %d\n", __FUNCTION__, width, height);

				if (modeID != (ULONG)INVALID_ID && result && width >= w*3 && height >= h /*&& width < 1280*/) {
					use_ham = 3;
				}
			}
			if (!use_ham) {
				modeID = INVALID_ID;
			}
		}

		if (modeID == (ULONG)INVALID_ID) {
			SDL_SetError("No high color modes are available"); // TODO better error messages
			return NULL;
		}

		ULONG table[]={1<<16, 0, 0, 0, 0};
		screen = OpenScreenTags(0,
			SA_DisplayID, modeID,
			SA_Width, use_ham ? w*use_ham : w,
			SA_Height, h,
			SA_Depth, use_ham ? 8 : 16,
			SA_ShowTitle, FALSE,
			SA_Quiet, TRUE,
			SA_Draggable, FALSE,
			SA_Type, CUSTOMSCREEN,
			SA_Colors32, (ULONG)table,
			TAG_DONE);

		BOOL contiguous = TRUE;
		if (use_ham) {
			struct BitMap *bm = screen->RastPort.BitMap;

			for (int i = 1; i < bm->Depth; i++) {
				ULONG diff = (ULONG)bm->Planes[i] - (ULONG)bm->Planes[i-1];
				if (diff != bm->BytesPerRow * bm->Rows) {
					contiguous = FALSE;
					break;
				}
			}

			if (!contiguous) {
				SDL_DestroyWindow(NULL);
				SDL_SetError("Couldn't allocate a contiguous bitmap");
			}
		}

		if (!screen) {
			if (contiguous) {
				SDL_SetError("Failed to open the custom screen");
			}
			return NULL;
		}
	}

	ULONG wflags = WFLG_ACTIVATE | WFLG_RMBTRAP;
	if (screen) {
		wflags |= WFLG_BACKDROP | WFLG_BORDERLESS;
	} else {
		wflags |=  WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;
	}

	window = OpenWindowTags(0,
		WA_InnerWidth, use_ham ? w*use_ham : w,
		WA_InnerHeight, h,
		screen ? TAG_IGNORE : WA_Title, (IPTR)title,
		WA_Flags, wflags,
		screen ? WA_CustomScreen : TAG_IGNORE, (IPTR)screen,
		!screen ? WA_PubScreenName : TAG_IGNORE, (IPTR)wndPubScreen,
		WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_RAWKEY,
		TAG_DONE);

	if (!window) {
		SDL_SetError("Failed to open the window");
		SDL_DestroyWindow(NULL);
		return NULL;
	}

	if (use_ham) {
		struct BitMap *bm = window->RPort->BitMap;
		if (use_ham == 4) {
			memset(bm->Planes[6], 0x77, bm->BytesPerRow * bm->Rows);
			memset(bm->Planes[7], 0xcc, bm->BytesPerRow * bm->Rows);
			c2p_2rgb565_4rgb555h8_040_init(w, h, 0, 0, bm->BytesPerRow, bm->BytesPerRow*bm->Rows, w*2);
		} else {
			int count = (bm->BytesPerRow >> 2) * bm->Rows;
			LONG *pixels = (LONG *)bm->Planes[6];
			for (int i = 0; i < count; i++) {
				*pixels++ = 0x6db6db6d;
			}
			pixels = (LONG *)bm->Planes[7];
			for (int i = 0; i < count; i++) {
				*pixels++ = 0xdb6db6db;
			}
			c2p_2rgb565_3rgb555h8_040_init(w, h, 0, 0, bm->BytesPerRow, bm->BytesPerRow*bm->Rows, w*2);
		}
	}

	if (screen) {
		pointermem = (UWORD *)AllocVec(2 * 6, MEMF_CHIP | MEMF_CLEAR);
		if (pointermem) {
			SetPointer(window, pointermem, 1, 1, 0, 0);
		}
	}

	sdlWindow.window = window;
	sdlWindow.w = w;
	sdlWindow.h = h;

	return &sdlWindow;
}

void SDL_DestroyWindow(SDL_Window *windowUNUSED)
{
	sdlWindow.window = NULL;
	if (window) {
		CloseWindow(window);
		window = NULL;
	}
	if (screen) {
		CloseScreen(screen);
		screen = NULL;
	}
	if (pointermem) {
		FreeVec(pointermem);
		pointermem = NULL;
	}
}

#if 0
int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		SDL_RWops* rw =  SDL_RWFromFile(argv[1], "r");
		if (rw)
		{
			SDL_Surface* loadedSurface = SDL_LoadBMP_RW(rw, 0);
			if (loadedSurface)
			{
				SDL_FreeSurface(loadedSurface);
			}
			SDL_RWclose(rw);
		}
	}
	return 0;
}

#endif
