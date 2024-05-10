#include <graphics/gfxbase.h>
#include <devices/audio.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <clib/alib_protos.h>

//#include <stdio.h>

#ifndef AUDIOTEST
#include "audiodev.h"
#endif

enum
{
	CHANNEL_LEFT,
	CHANNEL_RIGHT,
	CHANNEL_COUNT
};
#define LEFT_MASK 0x9
#define RIGHT_MASK 0x6

typedef struct
{
	struct MsgPort *port;
	struct IOAudio *ioreq;
	BYTE device;
} audiodev_t;

static BOOL audiodev_init(audiodev_t *dev, UBYTE *whichannel, ULONG chancount)
{
	if ((dev->port = CreateMsgPort()))
	{
		if ((dev->ioreq = (struct IOAudio *)CreateIORequest(dev->port, sizeof(struct IOAudio))))
		{
			struct IOAudio *audioReq = dev->ioreq;
			audioReq->ioa_Request.io_Message.mn_Node.ln_Pri = 127; // no stealing
			audioReq->ioa_Request.io_Command = ADCMD_ALLOCATE;
			audioReq->ioa_Request.io_Flags = ADIOF_NOWAIT;
			audioReq->ioa_AllocKey = 0;
			audioReq->ioa_Data = whichannel;
			audioReq->ioa_Length = chancount;
			if (!(dev->device = OpenDevice((STRPTR)AUDIONAME, 0, (struct IORequest *)audioReq, 0)))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

static void audiodev_close(audiodev_t *dev)
{
	if (!dev->device)
	{
		struct IOAudio *audioReq = dev->ioreq;
		//audioReq->ioa_Request.io_Command = CMD_RESET;
		audioReq->ioa_Request.io_Command = CMD_FLUSH;
		audioReq->ioa_Request.io_Flags = 0;
		DoIO((struct IORequest *)audioReq);
		CloseDevice((struct IORequest *)audioReq);
		dev->device = -1;
	}

	if (dev->ioreq)
	{
		DeleteIORequest((struct IORequest *)dev->ioreq);
		dev->ioreq = NULL;
	}

	if (dev->port)
	{
		DeleteMsgPort(dev->port);
		dev->port = NULL;
	}
}

static audiodev_t digidev;
static struct IOAudio digiChannelReq[2][CHANNEL_COUNT];
static LONG audioClock;
static UWORD audioPeriod;

static void channel_init(struct IOAudio *srcio, struct IOAudio *destio, ULONG chanmask)
{
	CopyMem(srcio, destio, sizeof(struct IOAudio));
	destio->ioa_Request.io_Unit = (struct Unit *)((ULONG)destio->ioa_Request.io_Unit & chanmask);
	destio->ioa_Request.io_Message.mn_ReplyPort = CreateMsgPort();
	destio->ioa_Request.io_Message.mn_Node.ln_Type = 0;
	destio->ioa_Length = 0;
	destio->ioa_Request.io_Command = CMD_WRITE;
	//destio->ioa_Request.io_Flags = ADIOF_PERVOL /*| IOF_QUICK*/;
}

static void channel_close(struct IOAudio *destio)
{
	//destio->ioa_Request.io_Command = CMD_RESET;
	destio->ioa_Request.io_Command = CMD_FLUSH;
	destio->ioa_Request.io_Flags = 0;
	DoIO((struct IORequest *)destio);
	if (destio->ioa_Request.io_Message.mn_ReplyPort)
	{
		DeleteMsgPort(destio->ioa_Request.io_Message.mn_ReplyPort);
		destio->ioa_Request.io_Message.mn_ReplyPort = NULL;
	}
}

int sound_init(int rate)
{
	//UBYTE whichannel[] = {3, 5, 10, 12};
	UBYTE whichannel[] = {15};
	ULONG leftmask[2] = {0x2, 0x4};
	ULONG rightmask[2] = {0x1, 0x8};

	if (audiodev_init(&digidev, whichannel, sizeof(whichannel)))
	{
		channel_init(digidev.ioreq, &digiChannelReq[0][CHANNEL_LEFT], leftmask[0]);
		channel_init(digidev.ioreq, &digiChannelReq[0][CHANNEL_RIGHT], rightmask[0]);
		channel_init(digidev.ioreq, &digiChannelReq[1][CHANNEL_LEFT], leftmask[1]);
		channel_init(digidev.ioreq, &digiChannelReq[1][CHANNEL_RIGHT], rightmask[1]);

		if (GfxBase->DisplayFlags & PAL)
			audioClock = 3546895;
		else
			audioClock = 3579545;

		audioPeriod = audioClock / rate;

		return 1;
	}
	audiodev_close(&digidev);

	return 0;
}

void sound_close(void)
{
	channel_close(&digiChannelReq[0][CHANNEL_LEFT]);
	channel_close(&digiChannelReq[0][CHANNEL_RIGHT]);
	channel_close(&digiChannelReq[1][CHANNEL_LEFT]);
	channel_close(&digiChannelReq[1][CHANNEL_RIGHT]);
	audiodev_close(&digidev);
}

static void channel_setup(struct IOAudio *audioio, UBYTE *data, ULONG length, UWORD period, UWORD volume, UWORD cycles)
{
	audioio->ioa_Data = data;
	audioio->ioa_Length = length;
	audioio->ioa_Period = period;
	audioio->ioa_Volume = volume;
	audioio->ioa_Cycles = cycles;
	audioio->ioa_Request.io_Flags = ADIOF_PERVOL | IOF_QUICK;
}

/*
static void channel_stop(struct IOAudio *audioio)
{
	if (audioio->ioa_Length == 0)
		return;

	if (!CheckIO((struct IORequest *)audioio))
	{
		AbortIO((struct IORequest *)audioio);
		WaitIO((struct IORequest *)audioio);
	}
}
*/

void sound_stop(int channel)
{
	struct IOAudio *audioReq = digidev.ioreq;
	struct IOAudio *digiLeftReq = &digiChannelReq[channel][CHANNEL_LEFT];
	struct IOAudio *digiRightReq = &digiChannelReq[channel][CHANNEL_RIGHT];
	struct Unit *oldunit = audioReq->ioa_Request.io_Unit;
	audioReq->ioa_Request.io_Unit = (struct Unit *)((ULONG)digiLeftReq->ioa_Request.io_Unit | (ULONG)digiRightReq->ioa_Request.io_Unit);
	audioReq->ioa_Request.io_Command = CMD_FLUSH;
	DoIO((struct IORequest *)audioReq);
	audioReq->ioa_Request.io_Unit = oldunit;
}

void sound_play(int channel, char *data, int length, int leftvol, int rightvol, int cycles)
{
	struct IOAudio *audioReq = digidev.ioreq;
	struct IOAudio *digiLeftReq = &digiChannelReq[channel][CHANNEL_LEFT];
	struct IOAudio *digiRightReq = &digiChannelReq[channel][CHANNEL_RIGHT];
	GetMsg(digiLeftReq->ioa_Request.io_Message.mn_ReplyPort);
	GetMsg(digiRightReq->ioa_Request.io_Message.mn_ReplyPort);
	struct Unit *oldunit = audioReq->ioa_Request.io_Unit;
	audioReq->ioa_Request.io_Unit = (struct Unit *)((ULONG)digiLeftReq->ioa_Request.io_Unit | (ULONG)digiRightReq->ioa_Request.io_Unit);
	audioReq->ioa_Request.io_Command = CMD_STOP;
	DoIO((struct IORequest *)audioReq);
	channel_setup(digiLeftReq, data, length, audioPeriod, leftvol, cycles);
	channel_setup(digiRightReq, data, length, audioPeriod, rightvol, cycles);
	BeginIO((struct IORequest *)digiLeftReq);
	BeginIO((struct IORequest *)digiRightReq);
	audioReq->ioa_Request.io_Command = CMD_START;
	DoIO((struct IORequest *)audioReq);
	audioReq->ioa_Request.io_Unit = oldunit;
}

/*
static void channel_volume(struct IOAudio *audioio, UWORD volume)
{
	audioio->ioa_Request.io_Command = ADCMD_PERVOL;
	audioio->ioa_Volume = volume;
	DoIO((struct IORequest *)audioio);
}

void sound_setvolume(int leftvol, int rightvol)
{
	struct IOAudio *digiLeftReq = &digiChannelReq[CHANNEL_LEFT];
	struct IOAudio *digiRightReq = &digiChannelReq[CHANNEL_RIGHT];
	struct IOAudio *audioReq = digidev.ioreq;
	struct Unit *oldunit = audioReq->ioa_Request.io_Unit;
	audioReq->ioa_Request.io_Unit = digiLeftReq->ioa_Request.io_Unit;
	audioReq->ioa_Period = digiLeftReq->ioa_Period; // TODO init this
	channel_volume(audioReq, leftvol);
	audioReq->ioa_Request.io_Unit = digiRightReq->ioa_Request.io_Unit;
	audioReq->ioa_Period = digiRightReq->ioa_Period; // TODO init this
	channel_volume(audioReq, rightvol);
	audioReq->ioa_Request.io_Unit = oldunit;
}
*/

static BOOL channel_playing(struct IOAudio *audioio)
{
	if (audioio->ioa_Length == 0)
		return 0;

	if (CheckIO((struct IORequest *)audioio))
	{
		WaitIO((struct IORequest *)audioio);
		return 0;
	}

	return 1;
}

int sound_isplaying(int channel)
{
	BOOL leftPlaying = channel_playing(&digiChannelReq[channel][CHANNEL_LEFT]);
	BOOL rightPlaying = channel_playing(&digiChannelReq[channel][CHANNEL_RIGHT]);
	return leftPlaying || rightPlaying;
}

#ifdef AUDIOTEST
#include "sound19.h"
int main(int argc, char *argv[])
{
	if (!sound_init(8000))
		return 1;

	Printf("Playing the sound...\n");
	sound_play(sound19_au, sound19_au_len, 0, 64, 64, 1);
	Printf("Waiting a second...\n");
	/*
	Delay(1*50);
	sound_stop();
	*/
	Printf("Waiting for 10 seconds...\n");
	Delay(10*50);

	sound_close();

	return 0;
}
#endif
