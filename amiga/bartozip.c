/*
* BarExtractor
* Copyright (C) 2019-2020 Erick Vasquez Garcia
* Copyright (C) 2024 Szilard Biro
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <conio.h>
#endif
#include <stdarg.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <dos.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <zlib.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>


#define CHUNK 1024

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
static int inf(FILE *source, FILE *dest)
{
	int ret;
	unsigned have;
	z_stream strm;
	static unsigned char in[CHUNK];
	static unsigned char out[CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	//ret = inflateInit(&strm);
	ret = inflateInit2(&strm, 16+MAX_WBITS); // GZip
	if (ret != Z_OK)
		return ret;

	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;	 /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret, FILE *source, FILE *dest)
{
	switch (ret) {
	case Z_ERRNO:
		if (ferror(source))
			fputs("error reading source\n", stderr);
		if (ferror(dest))
			fputs("error writing dest\n", stderr);
		break;
	case Z_STREAM_ERROR:
		fputs("invalid compression level\n", stderr);
		break;
	case Z_DATA_ERROR:
		fputs("invalid or incomplete deflate data\n", stderr);
		break;
	case Z_MEM_ERROR:
		fputs("out of memory\n", stderr);
		break;
	case Z_VERSION_ERROR:
		fputs("zlib version mismatch!\n", stderr);
	}
}

static char fread8(FILE *fp)
{
	char buffer[1];
	fread(buffer, 1, 1, fp);
	return buffer[0];
}

static short fread16be(FILE *fp)
{
	unsigned char buffer[2];
	fread(buffer, 2, 1, fp);
	return buffer[1] | buffer[0] << 8;
}

static short fread16le(FILE *fp)
{
	unsigned char buffer[2];
	fread(buffer, 2, 1, fp);
	return buffer[0] | buffer[1] << 8;
}

static int fread32be(FILE *fp)
{
	unsigned char buffer[4];
	fread(buffer, 4, 1, fp);
	return buffer[3] | buffer[2] << 8 | buffer[1] << 16 | buffer[0] << 24;
}

static int fread32le(FILE *fp)
{
	unsigned char buffer[4];
	fread(buffer, 4, 1, fp);
	return buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
}

static size_t fwrite8(char value, FILE *fp)
{
	return fwrite(&value, 1, 1, fp);
}

static size_t fwrite16le(int value, FILE *fp)
{
	unsigned char buffer[2];
	buffer[0] = value & 0xFF;
	buffer[1] = (value >> 8) & 0xFF;
	return fwrite(buffer, 2, 1, fp);
}

static size_t fwrite32le(int value, FILE *fp)
{
	unsigned char buffer[4];
	buffer[0] = value & 0xFF;
	buffer[1] = (value >> 8) & 0xFF;
	buffer[2] = (value >> 16) & 0xFF;
	buffer[3] = (value >> 24) & 0xFF;
	return fwrite(buffer, 4, 1, fp);
}

// TODO what is this stuff?
static unsigned char QCPDATA[] =
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x38,0x23,0x00,0xa0,0x00,0x40,0x1f,
	0x10,0x00,0x04,0x00,0x00,0x00,0x03,0x01,0x07,0x02,0x10,0x03,0x22,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
static const char qcelp_guid_1[16] = { 0x41, 0x6D, 0x7F, 0x5E, 0x15, 0xB1, 0xD0, 0x11, 0xBA, 0x91, 0x00, 0x80, 0x5F, 0xB4, 0xB9, 0x7E };

static int cmfextract(FILE *source, FILE *dest)
{
	unsigned char chunk[4];
	int ilen;
	short slen;
	//char clen;
	unsigned short ushort;
	unsigned char uchar;

	//fseek(dest, 0, SEEK_SET);
	memset(chunk, 0xFF, sizeof(chunk));
	fwrite("RIFF", 4, 1, dest);
	fwrite(chunk, 4, 1, dest); // file size

	fwrite("QLCM", 4, 1, dest);
	fwrite("fmt ", 4, 1, dest);
	fwrite32le(150, dest); // fmt size
	fwrite8(1, dest); // major
	fwrite8(0, dest); // major
	fwrite(qcelp_guid_1, sizeof(qcelp_guid_1), 1, dest);
	fwrite8(2, dest); // ???
	fwrite8(0, dest); // ???
	fwrite("Qcelp 13K", 9, 1, dest);

	fwrite(&QCPDATA, sizeof(QCPDATA), 1, dest);

	fwrite("vrat", 4, 1, dest);
	fwrite32le(8, dest); // vrat size
	fwrite32le(1, dest); // var-rate-flag
	fwrite(chunk, 4, 1, dest); // block_data size

	fwrite("data", 4, 1, dest);
	fwrite(chunk, 4, 1, dest); // data size
	long data_start = ftell(dest);

	fread(chunk, sizeof(chunk), 1, source);
	ilen = fread32be(source);
	//printf("file-length: %u\n", ilen);
	slen = fread16be(source);
	//printf("header-length: %u\n", slen);
	fseek(source, slen-4, SEEK_CUR);
	fread(chunk, sizeof(chunk), 1, source); // WAVE
	//printf("chunk: %.*s (%x %x %x %x)\n", 4, chunk, chunk[3], chunk[2], chunk[1], chunk[0]);
	fread(chunk, 4, 1, source); // trac
	//printf("chunk: %.*s (%x %x %x %x)\n", 4, chunk, chunk[3], chunk[2], chunk[1], chunk[0]);
	ilen = fread32be(source);
	//printf("buffer size: %u\n", ilen);

	//int buffer_size = ilen;
	fread(chunk, 1, 1, source); // 0xFF

	bool done = false;
	do
	{
		fread(chunk, 2, 1, source);
		uchar = chunk[1];
		//printf("code %x\n", uchar);
		switch (uchar)
		{
			default:
				printf("unknown code %x\n", uchar);
				exit(-1);
				//done = true;
				break;
			case 0xC3:
				fread(chunk, 2, 1, source);
				break;
			case 0xDE:
				fread(chunk, 2, 1, source);
				break;
			case 0xDF:
				fread(chunk, 1, 1, source);
				//printf("end marker: %u (0x%x)\n", *chunk, *chunk);
				done = true;
				break;
			case 0xE8:
				fread(chunk, 2, 1, source);
				break;
			case 0xF1:
				slen = fread16be(source);
				//printf("data size: %u\n", slen);

				ushort = fread16be(source); // type = 0x44
				//printf("type %x\n", ushort);

				ilen = fread32be(source);
				//printf("some other size: %u\n", ilen);

				fread(chunk, 1, 1, source); // pad
				//printf("pad: %u\n", *chunk);

				int data_size = slen-7;
				for (int i = 0; i < data_size; i++)
				{
					fread(&uchar, 1, 1, source);
					fwrite(&uchar, 1, 1, dest);
				}

				fread(chunk, 1, 1, source); // pad
				//printf("pad: %u\n", *chunk);
				//exit(-1);
				break;
		}
	} while(!done);

	long file_end = ftell(dest);
	int file_size = (file_end - 8);
	int data_size = (file_end - data_start);
	int block_size = data_size / 35; // TODO

	fseek(dest, 4, SEEK_SET);
	fwrite32le(file_size, dest); // file size
	fseek(dest, 174 /*+ 8*/, SEEK_CUR); // fmt + vrat size
	//printf("end: %ld\n", ftell(dest));
	fwrite32le(block_size, dest); // block_data size
	fseek(dest, 4, SEEK_CUR);
	fwrite32le(data_size, dest); // data size

	return 1;
}

static int rawtowav(FILE *source, FILE *dest)
{
	int data_size, startofs;
	int i;

	startofs = ftell(source);
	fseek(source, 0, SEEK_END);
	data_size = ftell(source);
	fseek(source, startofs, SEEK_SET);

	fwrite("RIFF", 4, 1, dest);
	fwrite32le(data_size + 44 - 8, dest); // file size
	fwrite("WAVE", 4, 1, dest);
	fwrite("fmt ", 4, 1, dest);
	fwrite32le(16, dest); // fmt size
	fwrite16le(1, dest); // Format code
	fwrite16le(1, dest); // Number of interleaved channels
	fwrite32le(8000, dest); // Sampling rate
	fwrite32le(8000*2, dest); // Data rate
	fwrite16le(2, dest); // Data block size
	fwrite16le(16, dest); // Bits per sample
	fwrite("data", 4, 1, dest);
	fwrite32le(data_size, dest);

	for (i = 0; i < data_size; i++)
	{
		unsigned char c = fread8(source);
		fwrite8(c, dest);
	}

	return 1;
}

int main(int argc, char *argv[])
{
	FILE *f1, *f2;
	int tableoffset;
	int filesize;
	int datasize;
	int tablecount;
	int zipsize;
	int i, j;
	int *table_data;
	char filename[256];
	short mimelen;
	short firstid;
	int ret;

	f1 = fopen("doomrpg.bar", "rb");
	if(f1 == NULL)
	{
		printf("Can't open doomrpg.bar\n");
		return 1;
	}

#ifdef _WIN32
	mkdir("unpacked");
#else
	mkdir("unpacked", 0777);
#endif

	fseek(f1,0x10,SEEK_SET);
	tableoffset = fread32le(f1);
	fseek(f1,0x1C,SEEK_SET);
	datasize = fread32le(f1);
	fseek(f1, 0x22, SEEK_SET); // TODO
	firstid = fread16le(f1);
	fseek(f1,0,SEEK_END);
	filesize = ftell(f1);
	fseek(f1,tableoffset,SEEK_SET);

	tablecount = ((filesize-datasize)-tableoffset)/sizeof(int);

	//printf("tableoffset: %d, datasize %d, tablecount %d\n", tableoffset, datasize, tablecount);

	table_data = (int *)malloc(sizeof(int) * tablecount+1);
	for(i = 0; i < tablecount; i++)
	{
		table_data[i] = fread32le(f1);
	}
	table_data[i] = table_data[i-1];

	for(i = 0; i < tablecount; i++)
	{
		zipsize = (table_data[i+1] - table_data[i]);
		//printf("zipsize %d\n",zipsize);
		if(zipsize <= 16)
			continue;

		// header stuff?
		fseek(f1,table_data[i],SEEK_SET);
		mimelen = fread16le(f1);
		fread(filename, mimelen - sizeof(short), 1, f1);
		//printf("id: %4d size %d\tMIME: %s (%d)\n", firstid + i, zipsize, buffer, mimelen);
		if (strcmp(filename, "application/x-gzip-compressed"))
		{
			sprintf(filename, "unpacked/%03d.wav", firstid + i);
			f2 = fopen(filename, "rb");
			if (f2 != NULL)
			{
				fclose(f2);
				printf("%s already exists\n", filename);
				continue;
			}

			unsigned char chunk[4];
			fread(chunk, 4, 1, f1);
			if (memcmp(chunk, "cmid", 4))
			{
				printf("not Compact Media Format (CMF)\n");
				continue;
			}
			fseek(f1, 4, SEEK_CUR); // skip file-length
			j = fread16be(f1);
			//printf("header-length: %u\n", j);
			fseek(f1, j-4, SEEK_CUR);
			fread(chunk, 4, 1, f1);
			//printf("chunk: %.*s\n", 4, chunk);
			if (memcmp(chunk, "WAVE", 4))
			{
				//printf("chunk: %.*s\n", 4, chunk);
				if (!memcmp(chunk, "SONG", 4))
				{
					// create a dummy MID file
					sprintf(filename, "unpacked/%03d.mid", firstid + i);
					f2 = fopen(filename,"wb");
					fclose(f2);
				}
				else
				{
					printf("not a wave file!\n");
				}
				continue;
			}

			// extract QCP
			fseek(f1,table_data[i]+mimelen,SEEK_SET);
			strcpy(filename, "unpacked/tmp.qcp");
			//printf("Extract %s\n",filename);
			f2 = fopen(filename,"wb");
			cmfextract(f1, f2);
			fclose(f2);

			// convert to raw
			ret = system("celp13k -i unpacked/tmp.qcp -o unpacked/tmp.raw -D -T 0");
			if (ret < 0)
			{
				printf("conversion failed\n");
				continue;
			}
			remove("unpacked/tmp.qcp");

			// convert to wav
			strcpy(filename, "unpacked/tmp.raw");
			//printf("Source data %s\n", filename);
			f2 = fopen(filename, "rb");
			sprintf(filename, "unpacked/%03d.wav", firstid + i);
			printf("Saving %s\n", filename);
			FILE *dest = fopen(filename,"wb");
			rawtowav(f2, dest);
			fclose(f2);
			fclose(dest);
			remove("unpacked/tmp.raw");
		}
		else
		{
			//copy name
			strcpy(filename, "unpacked/");
			fseek(f1,table_data[i]+mimelen+10,SEEK_SET);
			for(j = 0; j < zipsize-mimelen; j++)
			{
				fread(&filename[j+9],sizeof(char),1,f1);
				if(filename[j+9] == 0) break;
			}
			f2 = fopen(filename, "rb");
			if (f2 != NULL)
			{
				fclose(f2);
				printf("%s already exists\n", filename);
				continue;
			}

			printf("Extract %s\n",filename);
			f2 = fopen(filename,"wb");

			//copy data
			fseek(f1,table_data[i]+mimelen,SEEK_SET);
			int ret = inf(f1, f2);
			if (ret != Z_OK)
				zerr(ret, f1, f2);
			fclose(f2);
		}
	}

	fclose(f1);
	free(table_data);

	// put everything into the zip
	system("zip -jum0 DoomRPG.zip unpacked/*");

	printf("\nDoomRPG.zip was created, you can close this window.\n");

	return EXIT_SUCCESS;
}
