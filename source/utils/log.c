#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <dirent.h>
#include <sys/stat.h>
#include <zlib.h>
#include <malloc.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <sdcard/gcsd.h>
#include <ogc/usbstorage.h>
#include <di/di.h>
#include <ogc/dvd.h>
#include <iso9660.h>

#include "networkop.h"
#include "gcunzip.h"

#ifdef HW_RVL
	#include "mem2.h"
#endif

#define THREAD_SLEEP 100


size_t LogToFile(char *buffer)
{
    FILE * file;    
	size_t written = 0;
	char * filePath = "sd:/rvr_snes9x_debug.txt";

	file = fopen(filePath, "ab");
	written = fwrite(buffer, sizeof(char), strlen(buffer), file);
	fclose(file);
	
	return written;
}