#include "fat.h"

#define CONFIG_VERSION 3
#define CONFIG_VERSION_EXTROM 2
#define CONFIG_VERSION_ROMPATH 3

typedef struct
{
    char name[8];
    char long_name[16];
} kickstartTYPE;

typedef struct
{
    unsigned char lores;
    unsigned char hires;
} filterTYPE;

typedef struct
{
    unsigned char speed;
    unsigned char drives;
} floppyTYPE;

typedef struct
{
    unsigned char enabled;	// 0: Disabled, 1: Hard file, 2: MMC (entire card), 3-6: Partition 1-4 of MMC card
    unsigned char present;
    char name[8];
    char long_name[16];
} hardfileTYPE;

typedef struct
{
    char          id[8];
    unsigned long version;
    kickstartTYPE kickstart;
    filterTYPE    filter;
    unsigned char memory;
    unsigned char chipset;
    floppyTYPE    floppy;
    unsigned char disable_ar3;
    unsigned char enable_ide;
    unsigned char scanlines;
	unsigned char misc; // Contains extra settings, such as scandoubler
    hardfileTYPE  hardfile[2];
    unsigned char cpu;
	unsigned char fastram;	// Contains fast mem (bit 0 & 1) and turbo chipram (bit 7) settings.
	unsigned char kick13patch;
	unsigned char autofire;
    kickstartTYPE extrom;		// Added in V2 config
	unsigned char drivesounds;	// Added in V3 config
	unsigned char pad1;
	unsigned char pad2;
	unsigned char pad3;
	unsigned long kickdir;	// Directory of kick file.
	unsigned long extromdir;	// Directory of ext rom file.
	unsigned long hdfdir[4]; // Directory for HDF files.  Space for expansion to four devices.
    hardfileTYPE  secondaryhardfile[2]; // hardfile entries for potential secondary IDE devices.
} configTYPE;

extern fileTYPE file;	// Temporary file available for use by other modules, to avoid repeated memory usage.
						// Shouldn't be considered persistent.

extern configTYPE config; 
extern char DebugMode;

int UploadKickstart(unsigned long dir,char *name);
char UploadActionReplay();
void SetConfigurationFilename(int config);	// Set configuration filename by slot number
unsigned char LoadConfiguration(char *filename);	// Can supply NULL to use filename previously set by slot number
unsigned char SaveConfiguration(char *filename);	// Can supply NULL to use filename previously set by slot number
unsigned char ConfigurationExists(char *filename);
int ApplyConfiguration(char reloadkickstart,char applydrives);

