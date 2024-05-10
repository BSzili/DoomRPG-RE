#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
//#include <zlib.h>

#include "DoomRPG.h"
#include "Z_Zip.h"

zip_file_t zipFile;

#define ZIPDIR "Zip"

void openZipFile(const char* name, zip_file_t* zipFile)
{
	char command[64];
	snprintf(command, sizeof(command), "unzip -uo -d %s %s -x *.pmd", ZIPDIR, name); //  *.mid
	if (system(command) == -1) {
		DoomRPG_Error("cannot extract %s\n", name);
	}
}

void closeZipFile(zip_file_t* zipFile)
{
}

unsigned char* readZipFileEntry(const char* name, zip_file_t* zipFile, int* sizep)
{
	char path[64];
	snprintf(path, sizeof(path), "%s/%s", ZIPDIR, name);
	FILE *fp = fopen(path, "r");

	if (fp == NULL) {
		DoomRPG_Error("did not find the %s file in the zip file", name);
	}

	fseek(fp, 0, SEEK_END);
	*sizep = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	byte* cdata = SDL_malloc(*sizep);
	if (cdata) {
		fread(cdata, *sizep, 1, fp); // TODO check the return value
	}
	fclose(fp);

	return cdata;
}
