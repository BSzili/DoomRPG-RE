#ifndef NOTSDL_H
#define NOTSDL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// base types

typedef int32_t Sint32;
typedef int16_t Sint16;
typedef int8_t Sint8;
typedef uint32_t Uint32;
typedef uint16_t Uint16;
typedef uint8_t Uint8;
typedef int SDL_bool;


// Keyboard

typedef enum
{
    SDL_SCANCODE_UNKNOWN = 0,

    SDL_SCANCODE_A = 4,
    SDL_SCANCODE_B = 5,
    SDL_SCANCODE_C = 6,
    SDL_SCANCODE_D = 7,
    SDL_SCANCODE_E = 8,
    SDL_SCANCODE_F = 9,
    SDL_SCANCODE_G = 10,
    SDL_SCANCODE_H = 11,
    SDL_SCANCODE_I = 12,
    SDL_SCANCODE_J = 13,
    SDL_SCANCODE_K = 14,
    SDL_SCANCODE_L = 15,
    SDL_SCANCODE_M = 16,
    SDL_SCANCODE_N = 17,
    SDL_SCANCODE_O = 18,
    SDL_SCANCODE_P = 19,
    SDL_SCANCODE_Q = 20,
    SDL_SCANCODE_R = 21,
    SDL_SCANCODE_S = 22,
    SDL_SCANCODE_T = 23,
    SDL_SCANCODE_U = 24,
    SDL_SCANCODE_V = 25,
    SDL_SCANCODE_W = 26,
    SDL_SCANCODE_X = 27,
    SDL_SCANCODE_Y = 28,
    SDL_SCANCODE_Z = 29,

    SDL_SCANCODE_1 = 30,
    SDL_SCANCODE_2 = 31,
    SDL_SCANCODE_3 = 32,
    SDL_SCANCODE_4 = 33,
    SDL_SCANCODE_5 = 34,
    SDL_SCANCODE_6 = 35,
    SDL_SCANCODE_7 = 36,
    SDL_SCANCODE_8 = 37,
    SDL_SCANCODE_9 = 38,
    SDL_SCANCODE_0 = 39,

    SDL_SCANCODE_RETURN = 40,
    SDL_SCANCODE_ESCAPE = 41,
    SDL_SCANCODE_BACKSPACE = 42,
    SDL_SCANCODE_TAB = 43,
    SDL_SCANCODE_SPACE = 44,

    SDL_SCANCODE_MINUS = 45,
    SDL_SCANCODE_EQUALS = 46,
    SDL_SCANCODE_LEFTBRACKET = 47,
    SDL_SCANCODE_RIGHTBRACKET = 48,
    SDL_SCANCODE_BACKSLASH = 49,
    SDL_SCANCODE_NONUSHASH = 50,
    SDL_SCANCODE_SEMICOLON = 51,
    SDL_SCANCODE_APOSTROPHE = 52,
    SDL_SCANCODE_GRAVE = 53,
    SDL_SCANCODE_COMMA = 54,
    SDL_SCANCODE_PERIOD = 55,
    SDL_SCANCODE_SLASH = 56,

    SDL_SCANCODE_CAPSLOCK = 57,

    SDL_SCANCODE_F1 = 58,
    SDL_SCANCODE_F2 = 59,
    SDL_SCANCODE_F3 = 60,
    SDL_SCANCODE_F4 = 61,
    SDL_SCANCODE_F5 = 62,
    SDL_SCANCODE_F6 = 63,
    SDL_SCANCODE_F7 = 64,
    SDL_SCANCODE_F8 = 65,
    SDL_SCANCODE_F9 = 66,
    SDL_SCANCODE_F10 = 67,
    SDL_SCANCODE_F11 = 68,
    SDL_SCANCODE_F12 = 69,

    SDL_SCANCODE_PRINTSCREEN = 70,
    SDL_SCANCODE_SCROLLLOCK = 71,
    SDL_SCANCODE_PAUSE = 72,
    SDL_SCANCODE_INSERT = 73,
    SDL_SCANCODE_HOME = 74,
    SDL_SCANCODE_PAGEUP = 75,
    SDL_SCANCODE_DELETE = 76,
    SDL_SCANCODE_END = 77,
    SDL_SCANCODE_PAGEDOWN = 78,
    SDL_SCANCODE_RIGHT = 79,
    SDL_SCANCODE_LEFT = 80,
    SDL_SCANCODE_DOWN = 81,
    SDL_SCANCODE_UP = 82,

    SDL_SCANCODE_NUMLOCKCLEAR = 83,
    SDL_SCANCODE_KP_DIVIDE = 84,
    SDL_SCANCODE_KP_MULTIPLY = 85,
    SDL_SCANCODE_KP_MINUS = 86,
    SDL_SCANCODE_KP_PLUS = 87,
    SDL_SCANCODE_KP_ENTER = 88,
    SDL_SCANCODE_KP_1 = 89,
    SDL_SCANCODE_KP_2 = 90,
    SDL_SCANCODE_KP_3 = 91,
    SDL_SCANCODE_KP_4 = 92,
    SDL_SCANCODE_KP_5 = 93,
    SDL_SCANCODE_KP_6 = 94,
    SDL_SCANCODE_KP_7 = 95,
    SDL_SCANCODE_KP_8 = 96,
    SDL_SCANCODE_KP_9 = 97,
    SDL_SCANCODE_KP_0 = 98,
    SDL_SCANCODE_KP_PERIOD = 99,

    SDL_SCANCODE_NONUSBACKSLASH = 100,
    SDL_SCANCODE_APPLICATION = 101,
    SDL_SCANCODE_POWER = 102,
    SDL_SCANCODE_KP_EQUALS = 103,
    SDL_SCANCODE_F13 = 104,
    SDL_SCANCODE_F14 = 105,
    SDL_SCANCODE_F15 = 106,
    SDL_SCANCODE_F16 = 107,
    SDL_SCANCODE_F17 = 108,
    SDL_SCANCODE_F18 = 109,
    SDL_SCANCODE_F19 = 110,
    SDL_SCANCODE_F20 = 111,
    SDL_SCANCODE_F21 = 112,
    SDL_SCANCODE_F22 = 113,
    SDL_SCANCODE_F23 = 114,
    SDL_SCANCODE_F24 = 115,
    SDL_SCANCODE_EXECUTE = 116,
    SDL_SCANCODE_HELP = 117,
    SDL_SCANCODE_MENU = 118,
    SDL_SCANCODE_SELECT = 119,
    SDL_SCANCODE_STOP = 120,
    SDL_SCANCODE_AGAIN = 121,
    SDL_SCANCODE_UNDO = 122,
    SDL_SCANCODE_CUT = 123,
    SDL_SCANCODE_COPY = 124,
    SDL_SCANCODE_PASTE = 125,
    SDL_SCANCODE_FIND = 126,
    SDL_SCANCODE_MUTE = 127,
    SDL_SCANCODE_VOLUMEUP = 128,
    SDL_SCANCODE_VOLUMEDOWN = 129,

    SDL_SCANCODE_KP_COMMA = 133,
    SDL_SCANCODE_KP_EQUALSAS400 = 134,

    SDL_SCANCODE_INTERNATIONAL1 = 135,
    SDL_SCANCODE_INTERNATIONAL2 = 136,
    SDL_SCANCODE_INTERNATIONAL3 = 137,
    SDL_SCANCODE_INTERNATIONAL4 = 138,
    SDL_SCANCODE_INTERNATIONAL5 = 139,
    SDL_SCANCODE_INTERNATIONAL6 = 140,
    SDL_SCANCODE_INTERNATIONAL7 = 141,
    SDL_SCANCODE_INTERNATIONAL8 = 142,
    SDL_SCANCODE_INTERNATIONAL9 = 143,
    SDL_SCANCODE_LANG1 = 144,
    SDL_SCANCODE_LANG2 = 145,
    SDL_SCANCODE_LANG3 = 146,
    SDL_SCANCODE_LANG4 = 147,
    SDL_SCANCODE_LANG5 = 148,
    SDL_SCANCODE_LANG6 = 149,
    SDL_SCANCODE_LANG7 = 150,
    SDL_SCANCODE_LANG8 = 151,
    SDL_SCANCODE_LANG9 = 152,

    SDL_SCANCODE_ALTERASE = 153,
    SDL_SCANCODE_SYSREQ = 154,
    SDL_SCANCODE_CANCEL = 155,
    SDL_SCANCODE_CLEAR = 156,
    SDL_SCANCODE_PRIOR = 157,
    SDL_SCANCODE_RETURN2 = 158,
    SDL_SCANCODE_SEPARATOR = 159,
    SDL_SCANCODE_OUT = 160,
    SDL_SCANCODE_OPER = 161,
    SDL_SCANCODE_CLEARAGAIN = 162,
    SDL_SCANCODE_CRSEL = 163,
    SDL_SCANCODE_EXSEL = 164,

    SDL_SCANCODE_KP_00 = 176,
    SDL_SCANCODE_KP_000 = 177,
    SDL_SCANCODE_THOUSANDSSEPARATOR = 178,
    SDL_SCANCODE_DECIMALSEPARATOR = 179,
    SDL_SCANCODE_CURRENCYUNIT = 180,
    SDL_SCANCODE_CURRENCYSUBUNIT = 181,
    SDL_SCANCODE_KP_LEFTPAREN = 182,
    SDL_SCANCODE_KP_RIGHTPAREN = 183,
    SDL_SCANCODE_KP_LEFTBRACE = 184,
    SDL_SCANCODE_KP_RIGHTBRACE = 185,
    SDL_SCANCODE_KP_TAB = 186,
    SDL_SCANCODE_KP_BACKSPACE = 187,
    SDL_SCANCODE_KP_A = 188,
    SDL_SCANCODE_KP_B = 189,
    SDL_SCANCODE_KP_C = 190,
    SDL_SCANCODE_KP_D = 191,
    SDL_SCANCODE_KP_E = 192,
    SDL_SCANCODE_KP_F = 193,
    SDL_SCANCODE_KP_XOR = 194,
    SDL_SCANCODE_KP_POWER = 195,
    SDL_SCANCODE_KP_PERCENT = 196,
    SDL_SCANCODE_KP_LESS = 197,
    SDL_SCANCODE_KP_GREATER = 198,
    SDL_SCANCODE_KP_AMPERSAND = 199,
    SDL_SCANCODE_KP_DBLAMPERSAND = 200,
    SDL_SCANCODE_KP_VERTICALBAR = 201,
    SDL_SCANCODE_KP_DBLVERTICALBAR = 202,
    SDL_SCANCODE_KP_COLON = 203,
    SDL_SCANCODE_KP_HASH = 204,
    SDL_SCANCODE_KP_SPACE = 205,
    SDL_SCANCODE_KP_AT = 206,
    SDL_SCANCODE_KP_EXCLAM = 207,
    SDL_SCANCODE_KP_MEMSTORE = 208,
    SDL_SCANCODE_KP_MEMRECALL = 209,
    SDL_SCANCODE_KP_MEMCLEAR = 210,
    SDL_SCANCODE_KP_MEMADD = 211,
    SDL_SCANCODE_KP_MEMSUBTRACT = 212,
    SDL_SCANCODE_KP_MEMMULTIPLY = 213,
    SDL_SCANCODE_KP_MEMDIVIDE = 214,
    SDL_SCANCODE_KP_PLUSMINUS = 215,
    SDL_SCANCODE_KP_CLEAR = 216,
    SDL_SCANCODE_KP_CLEARENTRY = 217,
    SDL_SCANCODE_KP_BINARY = 218,
    SDL_SCANCODE_KP_OCTAL = 219,
    SDL_SCANCODE_KP_DECIMAL = 220,
    SDL_SCANCODE_KP_HEXADECIMAL = 221,

    SDL_SCANCODE_LCTRL = 224,
    SDL_SCANCODE_LSHIFT = 225,
    SDL_SCANCODE_LALT = 226,
    SDL_SCANCODE_LGUI = 227,
    SDL_SCANCODE_RCTRL = 228,
    SDL_SCANCODE_RSHIFT = 229,
    SDL_SCANCODE_RALT = 230,
    SDL_SCANCODE_RGUI = 231,

    SDL_NUM_SCANCODES = 256
} SDL_Scancode;

extern Uint8 keystate[SDL_NUM_SCANCODES];
#define SDL_GetKeyboardState(numkeys) keystate
const char *SDL_GetScancodeName(SDL_Scancode scancode);

// Endianness

#define SDL_LIL_ENDIAN  1234
#define SDL_BIG_ENDIAN  4321
#ifdef __AMIGA__
#define SDL_BYTEORDER   SDL_BIG_ENDIAN
#else
#define SDL_BYTEORDER   SDL_LIL_ENDIAN
#endif

#include <stdint.h>

#define SDL_Swap16(x) (uint16_t)(((uint16_t)(x) & 0x00ff) << 8 | ((uint16_t)(x) & 0xff00) >> 8)
#define SDL_Swap32(x) (uint32_t)(((uint32_t)(x) & 0x000000ff) << 24 | ((uint32_t)(x) & 0x0000ff00) <<  8 | \
                                 ((uint32_t)(x) & 0x00ff0000) >>  8 | ((uint32_t)(x) & 0xff000000) >> 24)

static inline float SDL_SwapFloat(float x)
{
	union
	{
		float f;
		uint32_t ui32;
	} swapper;
	swapper.f = x;
	swapper.ui32 = SDL_Swap32(swapper.ui32);
	return swapper.f;
}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define SDL_SwapLE16(X)     (X)
#define SDL_SwapLE32(X)     (X)
#define SDL_SwapFloatLE(X)  (X)
#define SDL_SwapBE16(X)     SDL_Swap16(X)
#define SDL_SwapBE32(X)     SDL_Swap32(X)
#define SDL_SwapFloatBE(X)  SDL_SwapFloat(X)
#else
#define SDL_SwapLE16(X)     SDL_Swap16(X)
#define SDL_SwapLE32(X)     SDL_Swap32(X)
#define SDL_SwapFloatLE(X)  SDL_SwapFloat(X)
#define SDL_SwapBE16(X)     (X)
#define SDL_SwapBE32(X)     (X)
#define SDL_SwapFloatBE(X)  (X)
#endif


const char *SDL_GetError(void);
int SDL_SetError(const char *fmt, ...);
void SDL_ClearError(void);

#define SDL_InitSubSystem(flags) (0)
//#define SDL_GetTicks() (0)
#define SDL_INIT_EVERYTHING (0)

Uint32 SDL_GetTicks(void);
int SDL_Init(Uint32 flags);
void SDL_Quit(void);


// video

typedef struct SDL_Rect
{
	int x, y;
	int w, h;
} SDL_Rect;

typedef struct SDL_Point
{
	int x;
	int y;
} SDL_Point;

typedef struct SDL_Color
{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	Uint8 b;
	Uint8 g;
	Uint8 r;
	Uint8 a;
#else
	Uint8 a;
	Uint8 r;
	Uint8 g;
	Uint8 b;
#endif
} SDL_Color;

typedef struct SDL_Palette
{
    int ncolors;
    SDL_Color *colors;
    //Uint32 version;
    //int refcount;
} SDL_Palette;

typedef struct SDL_PixelFormat
{
	//Uint32 format;
	SDL_Palette *palette;
	/*
	Uint8 BitsPerPixel;
	*/
	//Uint8 BytesPerPixel; // TODO remove!
	/*
	Uint8 padding[2];
	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	Uint32 Amask;
	Uint8 Rloss;
	Uint8 Gloss;
	Uint8 Bloss;
	Uint8 Aloss;
	Uint8 Rshift;
	Uint8 Gshift;
	Uint8 Bshift;
	Uint8 Ashift;
	*/
	//int refcount;
	//struct SDL_PixelFormat *next;
} SDL_PixelFormat;

#define SDL_PIXELFORMAT_RGB565 (0) // hack
#define SDL_BYTESPERPIXEL(x) (2) // hack

typedef struct SDL_Surface
{
	SDL_PixelFormat *format;
	int w, h;
	int pitch;
	void *pixels;
	// private stuff
	Uint32 colorkey;
} SDL_Surface;

typedef enum
{
	SDL_WINDOW_FULLSCREEN = 0x00000001,
	SDL_WINDOW_OPENGL = 0x00000002,
	SDL_WINDOW_SHOWN = 0x00000004,
	SDL_WINDOW_RESIZABLE = 0x00000020,
	SDL_WINDOW_ALLOW_HIGHDPI = 0x00002000,
} SDL_WindowFlags;

#define SDL_WINDOWPOS_UNDEFINED (-1) // hack

// file I/O

#define RW_SEEK_SET SEEK_SET
#define RW_SEEK_CUR SEEK_CUR
#define RW_SEEK_END SEEK_END


#define SDL_RWOPS_STDFILE   2U
#define SDL_RWOPS_MEMORY    4U

typedef struct SDL_RWops
{
	Uint32 type;
	union
	{
		struct
		{
			SDL_bool autoclose;
			FILE *fp;
		} stdio;
		struct
		{
			Uint8 *base;
			Uint8 *here;
			Uint8 *stop;
		} mem;
	} hidden;
} SDL_RWops;

SDL_RWops *SDL_RWFromMem(void *mem, int size);
SDL_RWops *SDL_RWFromFile(const char *file, const char *mode);
int SDL_RWsize(struct SDL_RWops *context);
int SDL_RWseek(struct SDL_RWops *context, long offset, int whence);
int SDL_RWtell(struct SDL_RWops *context);
size_t SDL_RWread(SDL_RWops *context, void *ptr, size_t size, size_t maxnum);
size_t SDL_RWwrite(struct SDL_RWops *context, const void *ptr, size_t size, size_t num);
int SDL_RWclose(struct SDL_RWops *context);
Uint32 SDL_ReadLE32(SDL_RWops *src);
Uint16 SDL_ReadLE16(SDL_RWops * src);

SDL_Surface *SDL_LoadBMP_RW(SDL_RWops *src, int freesrc);
void SDL_FreeSurface(SDL_Surface *surface);
int SDL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key);
Uint32 SDL_MapRGB(const SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b);

// renderer

typedef enum
{
    SDL_RENDERER_SOFTWARE = 0x00000001,
    SDL_RENDERER_ACCELERATED = 0x00000002,
} SDL_RendererFlags;

typedef enum
{
	SDL_BLENDMODE_NONE = 0x00000000,
	SDL_BLENDMODE_BLEND = 0x00000001,
} SDL_BlendMode;

#define SDL_TEXTUREACCESS_STREAMING (0) // hack

struct SDL_Texture;
typedef struct SDL_Texture SDL_Texture;

struct SDL_Window;
typedef struct SDL_Window SDL_Window;

struct SDL_Renderer;
typedef struct SDL_Renderer SDL_Renderer;

SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags);
void SDL_DestroyRenderer(SDL_Renderer *renderer);
int SDL_RenderSetClipRect(SDL_Renderer *renderer, const SDL_Rect *rect);
void SDL_RenderPresent(SDL_Renderer *renderer);
int SDL_RenderClear(SDL_Renderer *renderer);
int SDL_SetRenderDrawColor(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
int SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect);
int SDL_RenderFillRect(SDL_Renderer *renderer, const SDL_Rect *rect);
int SDL_RenderDrawRect(SDL_Renderer *renderer, const SDL_Rect *rect);
int SDL_RenderDrawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2);
int SDL_RenderDrawPoints(SDL_Renderer *renderer, const SDL_Point *points, int count);
int SDL_SetRenderDrawBlendMode(SDL_Renderer *renderer, SDL_BlendMode blendMode);
#define SDL_RenderSetLogicalSize(x, y, z)

SDL_Texture *SDL_CreateTexture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h);
void SDL_DestroyTexture(SDL_Texture *texture);
int SDL_UpdateTexture(SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch);
int SDL_SetTextureColorMod(SDL_Texture *texture, Uint8 r, Uint8 g, Uint8 b);
SDL_Texture *SDL_CreateTextureFromSurface(SDL_Renderer *renderer, SDL_Surface *surface);

// controller

typedef enum
{
	SDL_CONTROLLER_BUTTON_INVALID = -1,
	SDL_CONTROLLER_BUTTON_A,
	SDL_CONTROLLER_BUTTON_B,
	SDL_CONTROLLER_BUTTON_X,
	SDL_CONTROLLER_BUTTON_Y,
	SDL_CONTROLLER_BUTTON_BACK,
	SDL_CONTROLLER_BUTTON_GUIDE,
	SDL_CONTROLLER_BUTTON_START,
	SDL_CONTROLLER_BUTTON_LEFTSTICK,
	SDL_CONTROLLER_BUTTON_RIGHTSTICK,
	SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
	SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
	SDL_CONTROLLER_BUTTON_DPAD_UP,
	SDL_CONTROLLER_BUTTON_DPAD_DOWN,
	SDL_CONTROLLER_BUTTON_DPAD_LEFT,
	SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
	SDL_CONTROLLER_BUTTON_MAX
} SDL_GameControllerButton;

typedef enum
{
	SDL_CONTROLLER_AXIS_INVALID = -1,
	SDL_CONTROLLER_AXIS_LEFTX,
	SDL_CONTROLLER_AXIS_LEFTY,
	SDL_CONTROLLER_AXIS_RIGHTX,
	SDL_CONTROLLER_AXIS_RIGHTY,
	SDL_CONTROLLER_AXIS_TRIGGERLEFT,
	SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
	SDL_CONTROLLER_AXIS_MAX
} SDL_GameControllerAxis;

typedef struct SDL_GameController
{
	void *handle;
} SDL_GameController;

typedef struct SDL_Joystick
{
	void *handle;
} SDL_Joystick;

typedef struct SDL_Haptic
{
	void *handle;
} SDL_Haptic;

#define SDL_NumJoysticks() (1)
#define SDL_IsGameController(i) (1)
#define SDL_GameControllerHasRumble(x) (1) // shut up the warning
#define SDL_GameControllerOpen(i) ((void*)1)
#define SDL_GameControllerClose(x)
//#define SDL_GameControllerGetButton(x, button) (0)
Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller, SDL_GameControllerButton button);
#define SDL_GameControllerGetAxis(x, y) (0)

#define SDL_JoystickOpen(i) NULL
#define SDL_JoystickClose(i)
#define SDL_JoystickIsHaptic(i) (0)
#define SDL_HapticOpenFromJoystick(x) NULL
#define SDL_HapticRumbleInit(x) (-1)
#define SDL_JoystickGetButton(x,y) (0)
#define SDL_JoystickGetAxis(x,y) (0)
#define SDL_JoystickNumAxes(x) (0)
#define SDL_HapticClose(x)

// message box

typedef enum
{
	SDL_MESSAGEBOX_ERROR = 0x00000010,
	SDL_MESSAGEBOX_WARNING = 0x00000020,
	SDL_MESSAGEBOX_INFORMATION = 0x00000040
} SDL_MessageBoxFlags;

typedef struct
{
	Uint32 flags;
	int buttonid;
	const char *text;
} SDL_MessageBoxButtonData;

typedef struct
{
	Uint8 r, g, b;
} SDL_MessageBoxColor;

typedef enum
{
	SDL_MESSAGEBOX_COLOR_BACKGROUND,
	SDL_MESSAGEBOX_COLOR_TEXT,
	SDL_MESSAGEBOX_COLOR_BUTTON_BORDER,
	SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND,
	SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED,
	SDL_MESSAGEBOX_COLOR_MAX
} SDL_MessageBoxColorType;

typedef struct
{
	SDL_MessageBoxColor colors[SDL_MESSAGEBOX_COLOR_MAX];
} SDL_MessageBoxColorScheme;

typedef struct
{
	Uint32 flags;
	SDL_Window *window;
	const char *title;
	const char *message;

	int numbuttons;
	const SDL_MessageBoxButtonData *buttons;

	const SDL_MessageBoxColorScheme *colorScheme;
} SDL_MessageBoxData;

int SDL_ShowMessageBox(const SDL_MessageBoxData *messageboxdata, int *buttonid);

// event handler

#define SDL_BUTTON(X)       (1 << ((X)-1))
#define SDL_BUTTON_LEFT     1
#define SDL_BUTTON_MIDDLE   2
#define SDL_BUTTON_RIGHT    3
#define SDL_BUTTON_X1       4
#define SDL_BUTTON_X2       5
#define SDL_BUTTON_LMASK    SDL_BUTTON(SDL_BUTTON_LEFT)
#define SDL_BUTTON_MMASK    SDL_BUTTON(SDL_BUTTON_MIDDLE)
#define SDL_BUTTON_RMASK    SDL_BUTTON(SDL_BUTTON_RIGHT)
#define SDL_BUTTON_X1MASK   SDL_BUTTON(SDL_BUTTON_X1)
#define SDL_BUTTON_X2MASK   SDL_BUTTON(SDL_BUTTON_X2)

typedef enum
{
	SDL_WINDOWEVENT_RESIZED,
	SDL_WINDOWEVENT_CLOSE,
} SDL_WindowEventID;

typedef enum
{
	SDL_QUIT = 0x100,
	SDL_WINDOWEVENT = 0x200,
	SDL_MOUSEMOTION = 0x400,
	SDL_MOUSEBUTTONDOWN,
	SDL_MOUSEBUTTONUP,
	SDL_MOUSEWHEEL,
} SDL_EventType;

typedef struct SDL_WindowEvent
{
	Uint32 windowID;
	Uint8 event;
} SDL_WindowEvent;

typedef struct SDL_MouseWheelEvent
{
	Sint32 x;
	Sint32 y;
} SDL_MouseWheelEvent;

typedef union SDL_Event
{
	Uint32 type;
	SDL_WindowEvent window;
	SDL_MouseWheelEvent wheel;
} SDL_Event;

#define SDL_SetRelativeMouseMode(x)
#define SDL_ShowCursor(x)
#define SDL_GetMouseState(x,y) (0)
#define SDL_GetRelativeMouseState(x,y)
#define SDL_WarpMouseInWindow(x,y,z)
#define SDL_GetWindowSize(x,y,z)

int SDL_PollEvent(SDL_Event *event);
SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags);
void SDL_DestroyWindow(SDL_Window *window);
#define SDL_SetWindowMinimumSize(x, y, z)
#define SDL_RenderSetIntegerScale(x,y)
int SDL_SetWindowFullscreen(SDL_Window *window, Uint32 flags);

// memory allocation
/**/
typedef void *(* SDL_malloc_func) (size_t size);
typedef void *(* SDL_calloc_func) (size_t nmemb, size_t size);
typedef void *(* SDL_realloc_func) (void *mem, size_t size);
typedef void(* SDL_free_func) (void *mem);
/**/

// these just wrap the native C library

#define SDL_malloc malloc
#define SDL_calloc calloc
#define SDL_realloc realloc
#define SDL_free free
#define SDL_memset memset
#define SDL_memmove memmove
#define SDL_memcpy memcpy
#define SDL_memcmp memcmp

#define SDL_snprintf snprintf
#define SDL_Log printf

#define SDL_strlen strlen
#define SDL_strcmp strcmp
#define SDL_strncmp strncmp
#define SDL_strchr strchr
#define SDL_strcasecmp strcasecmp
#define SDL_atoi atoi

#define SDL_arraysize(x) (sizeof((x)) / sizeof((x)[0]))

#define SDLCALL

#define SDL_TRUE (1)
#define SDL_FALSE (0)
#define SDL_DISABLE (0)
#define SDL_ENABLE (1)

// stubs

//#define SDL_HINT_RENDER_VSYNC "SDL_RENDER_VSYNC"
#define SDL_SetHint(x,y)
#define SDL_GameControllerRumble(x,y,z,w)
#define SDL_HapticRumbleStop(x)
#define SDL_HapticRumblePlay(x, y, z)
#define SDL_assert(x)

#endif

