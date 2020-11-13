#include "errors.h"
#include "hardware.h"
#include "mmc.h"
#include "fat.h"
#include "osd.h"
#include "fpga.h"
#include "fdd.h"
#include "hdd.h"
#include "firmware.h"
#include "menu.h"
#include "config.h"

#include "hexdump.h"

#include <stdio.h>
#include <string.h>

#define EXTENDED_ROM

configTYPE config;
fileTYPE file;
extern char s[40];
char configfilename[12];
char DebugMode;

int CheckSum();


unsigned char romkey[3072];

RAFile romfile;

static char filename[12];

void ClearKickstartMirrorE0(void)
{
	int i;
	int *p=(0xe00000 & 0x7fffff) ^ HOSTMAP_ADDR;
	for (i = 0; i < (0x80000 / 4); i++) {
		*p++=0;
	}
}

void ClearVectorTable(void)
{
	int i;
	int *p=0 ^ HOSTMAP_ADDR;
	for (i = 0; i < 256; i++) {
		*p++=0;
	}
}

//// UploadKickstart() ////
int UploadKickstart(unsigned long dir,char *name)
{
	int keysize=0;
	char filename[12];

	strncpy(filename, name, 8); // copy base name
	strcpy(&filename[8], "ROM"); // add extension

	printf("ROM dir: %d\n",dir);

	if(!ValidateDirectory(dir))
	{
		SetError(ERROR_ROM,"Bad ROM Directory",dir,0);
		return(0);
	}
	ChangeDirectory(dir);

	BootPrint("Checking for Amiga Forever key file:");
	if(RAOpen(&romfile,"ROM     KEY")) {
		keysize=romfile.size;
		if(romfile.size<sizeof(romkey)) {
			RARead(&romfile, &romkey[0],romfile.size);
		} else {
			SetError(ERROR_ROM,"ROM Key file wrong size",romfile.size,0);
		}
	}

	if (RAOpen(&romfile, filename)) {
		ClearError(ERROR_FILESYSTEM);
		if ((romfile.size & 0xf) == 0xb && !keysize) {
			FatalError(ERROR_ROM,"ROM requires key file",0,0);
		}
		else if (romfile.size == 0x100000) {
			// 1MB Kickstart ROM
			BootPrint("Uploading 1MB Kickstart ...");
			SendFileV2(&romfile, NULL, 0, 0xe00000, romfile.size>>10);
			SendFileV2(&romfile, NULL, 0, 0xf80000, romfile.size>>10);
			ClearVectorTable();
			return(1024);
		} else if(romfile.size == 0x80000) {
			// 512KB Kickstart ROM
			SendFileV2(&romfile, NULL, 0, 0xf80000, romfile.size>>9);
			RAOpen(&romfile, filename);
			SendFileV2(&romfile, NULL, 0, 0xe00000, romfile.size>>9);
			ClearVectorTable();
			return(512);
		} else if ((romfile.size == 0x8000b)) {
			// 512KB Kickstart ROM
			SendFileV2(&romfile, romkey, keysize, 0xf80000, romfile.size>>9);
			RAOpen(&romfile, filename);
			SendFileV2(&romfile, romkey, keysize, 0xe00000, romfile.size>>9);
			ClearVectorTable();
			return(512);
		} else if (romfile.size == 0x40000) {
			// 256KB Kickstart ROM
			SendFileV2(&romfile, NULL, 0, 0xf80000, romfile.size>>9);
			RAOpen(&romfile, filename);
			SendFileV2(&romfile, NULL, 0, 0xfc0000, romfile.size>>9);
			ClearVectorTable();
			ClearKickstartMirrorE0();
			return(256);
		} else if ((romfile.size == 0x4000b)) {
			// 256KB Kickstart ROM
			SendFileV2(&romfile, romkey, keysize, 0xf80000, romfile.size>>9);
			RAOpen(&romfile, filename);
			SendFileV2(&romfile, romkey, keysize, 0xfc0000, romfile.size>>9);
			ClearVectorTable();
			ClearKickstartMirrorE0();
			return(256);
		} else {
			FatalError(ERROR_ROM,"ROM size incorrect",romfile.size,0);
		}
	} else {
		FatalError(ERROR_ROM,"ROM missing",0,0);
	}
	return(0);
}


// Upload Extended ROM
char UploadExtROM(unsigned long dir,char *name)
{
#ifdef EXTENDED_ROM
	char filename[12];

	strncpy(filename, name, 8); // copy base name
	strcpy(&filename[8], "ROM"); // add extension

	if(ValidateDirectory(dir))
	{
		ChangeDirectory(dir);
		if (RAOpen(&romfile, filename)) {
			ClearError(ERROR_FILESYSTEM);
			if(romfile.size == 0x80000) {
				// 512KB Extended CD32 ROM
				SendFileV2(&romfile, NULL, 0, 0xe00000, romfile.size>>9);
				return(1);
			}
			else
			{
				FatalError(ERROR_ROM,"ExtROM size incorrect",romfile.size,0);
			}
		}
	}
	else
		FatalError(ERROR_ROM,"Bad ExtROM directory",dir,0);
	return(0);
#else
	return(1);
#endif
}


//// UploadActionReplay() ////
char UploadActionReplay()
{
	// FIXME - migrate to direct access

	if (RAOpen(&romfile, "HRTMON  ROM")) {
		unsigned char *adr;
		int data;
		printf("Uploading HRTmon ROM... ");
		SendFileV2(&romfile, NULL, 0, 0xa10000, (romfile.file.size+511)>>9);

		// HRTmon config
		adr = (unsigned char *)(((0xa10000 + 20)&0x7fffff)^HOSTMAP_ADDR);

		*adr++=0x00; *adr++=0x80; *adr++=0x00; *adr++=0x00;
		//      data = 0x00800000; // mon_size, 4 bytes
		*adr++ =0x00; // col0h, 1 byte
		*adr++ =0x5a; // col0l, 1 byte
		*adr++ =0x0f; // col1h, 1 byte
		*adr++ =0xff; // col1l, 1 byte
		*adr++ =0xff; // right, 1 byte
		*adr++ =0x00; // keyboard, 1 byte
		*adr++ =0xff; // key, 1 byte
		*adr++ =config.enable_ide ? 0xff : 0; // ide, 1 byte
		*adr++ =0xff; // a1200, 1 byte
		*adr++ =config.chipset&CONFIG_AGA ? 0xff : 0; // aga, 1 byte
		*adr++ =0xff; // insert, 1 byte
		*adr++ =0x0f; // delay, 1 byte
		*adr++ =0xff; // lview, 1 byte
		*adr++ =0x00; // cd32, 1 byte
		*adr++ =config.chipset&CONFIG_NTSC ? 1 : 0; // screenmode, 1 byte
		*adr++ =0xff; // novbr, 1 byte
		*adr++ =0; // entered, 1 byte
		*adr++ =1; // hexmode, 1 byte

		adr = (unsigned char *)(((0xa10000 + 68) & 0x7fffff) ^ HOSTMAP_ADDR);
		data = ((config.memory&0x3) + 1) * 512 * 1024; // maxchip, 4 bytes TODO is this correct?
		*adr++=(data>>24)&0xff;
		*adr++=(data>>16)&0xff;
		*adr++=(data>>8)&0xff;
		*adr++=(data>>0)&0xff;

		return(1);
	} else {
	  ClearError(ERROR_FILESYSTEM);
	  puts("\rhrtmon.rom not found!\r");
	  return(0);
	}
  return(0);
}


void SetConfigurationFilename(int config)
{
	if(config)
		sprintf(configfilename,"MINMGAA%dCFG",config);
	else
		strcpy(configfilename,"MINMGAA CFG");
}



unsigned char ConfigurationExists(char *filename)
{
	if(!filename)
		filename=configfilename;	// Use slot-based filename if none provided.
	ChangeDirectory(0); // Config files always live in the root directory
    if (FileOpen(&file, filename))
    {
		return(1);
	}
	ClearError(ERROR_FILESYSTEM);
	return(0);
}


static const char config_id[] = "MNMGCFGA"; /* New signature for AGA core */

unsigned char LoadConfiguration(char *filename)
{
	int updatekickstart=0;
	int result=0;
    unsigned int key;

	if(!filename)
		filename=configfilename;	// Use slot-based filename if none provided.

    // load configuration data
	ChangeDirectory(0); // Config files always live in the root directory
    if (FileOpen(&file, filename))
    {
		configTYPE *tmpconf=(configTYPE *)&sector_buffer;
		BootPrint("Opened configuration file\n");
        printf("Configuration file size: %lu\r", file.size);
        if (file.size <= sizeof(config))
        {
            FileRead(&file, sector_buffer);

			// A few more sanity checks...
			if(tmpconf->floppy.drives<=4) 
			{
				// If either the old config and new config have a different kickstart file,
				// or this is the first boot, we need to upload a kickstart image.
				if(strncmp(tmpconf->kickstart.name,config.kickstart.name,8)!=0)
					updatekickstart=true;
				result=1; // We successfully loaded the config.
			}
			else
				BootPrint("Config file sanity check failed!\n");

            // check file id and version
            if (strncmp(tmpconf->id, config_id, sizeof(config.id)) != 0)
				result=0;

            if(tmpconf->version<CONFIG_VERSION_EXTROM)
			{
				BootPrint("Config file predates ExtROM support");
				strncpy(tmpconf->extrom.name, "EXTENDED", sizeof(config.extrom.name));
			}
            if(tmpconf->version<CONFIG_VERSION_ROMPATH)
			{
				BootPrint("Config file predates ROM Path support");
				tmpconf->kickdir=0;
				tmpconf->hdfdir[0]=tmpconf->hdfdir[1]=tmpconf->hdfdir[2]=tmpconf->hdfdir[3]=0;
				tmpconf->secondaryhardfile[0].enabled=0;
				tmpconf->secondaryhardfile[0].present=0;
				tmpconf->secondaryhardfile[0].name[0]=0;
				tmpconf->secondaryhardfile[0].long_name[0]=0;
				tmpconf->secondaryhardfile[1].enabled=0;
				tmpconf->secondaryhardfile[1].present=0;
				tmpconf->secondaryhardfile[1].name[0]=0;
				tmpconf->secondaryhardfile[1].long_name[0]=0;
			}
			if(tmpconf->version>CONFIG_VERSION)
			{
				BootPrint("Wrong configuration file format!\n");
				result=0;
			}

			tmpconf->version=CONFIG_VERSION;
			if(result)
                memcpy((void*)&config, (void*)sector_buffer, sizeof(config));
        }
    }
    if(!result)
	{
        BootPrint("Config loading failed - using defaults\n");
		ClearError(ERROR_FILESYSTEM);
		// set default configuration
		memset((void*)&config, 0, sizeof(config));	// Finally found default config bug - params were reversed!
		strncpy(config.id, config_id, sizeof(config.id));
		strncpy(config.kickstart.name, "KICK    ", sizeof(config.kickstart.name));
		config.version = CONFIG_VERSION;
		config.misc = 1<<(PLATFORM_SCANDOUBLER);  // platform register - enable scandoubler by default
		config.kickstart.long_name[0] = 0;
		config.memory = 0x15;
		config.cpu = 0;
		config.chipset = 0;
		config.floppy.speed=CONFIG_FLOPPY2X;
		config.floppy.drives=1;
		config.enable_ide=0;
		config.hardfile[0].enabled = 1;
		strncpy(config.hardfile[0].name, "HARDFILE", sizeof(config.hardfile[0].name));
		config.hardfile[0].long_name[0]=0;
		strncpy(config.hardfile[1].name, "HARDFILE", sizeof(config.hardfile[1].name));
		config.hardfile[1].long_name[0]=0;
		config.hardfile[1].enabled = 2;	// Default is access to entire SD card
		updatekickstart=true;

		/* Version 2 configuration fields */
		strncpy(config.extrom.name, "EXTENDED", sizeof(config.extrom.name));

		BootPrint("Defaults set\n");
	}

    return(result);
}


int ApplyConfiguration(char reloadkickstart, char applydrives)
{
	int rstval=0;
	int result=0;
	int romsize=0;
//	printf("c1: %x\n",CheckSum());

	// Whether or not we uploaded a kickstart image we now need to set various parameters from the config.

	if(applydrives)
	{
		if(OpenHardfile(0))
		{
		//		printf("c2: %x\n",CheckSum());
			switch(hdf[0].type) // Customise message for SD card access
			{
				case (HDF_FILE | HDF_SYNTHRDB):
					sprintf(s, "\nHardfile 1 (with fake RDB): %.8s.%.3s", hdf[1].file.name, &hdf[1].file.name[8]);
					break;
				case HDF_FILE:
					sprintf(s, "\nHardfile 0: %.8s.%.3s", hdf[0].file.name, &hdf[0].file.name[8]);
					break;
				case HDF_CARD:
					sprintf(s, "\nHardfile 0: using entire SD card");
					break;
				default:
					sprintf(s, "\nHardfile 0: using SD card partition %d",hdf[0].type-HDF_CARD);	// Number from 1
					break;
			}
			BootPrint(s);
			sprintf(s, "CHS: %u.%u.%u", hdf[0].cylinders, hdf[0].heads, hdf[0].sectors);
			BootPrint(s);
			sprintf(s, "Size: %lu MB", ((((unsigned long) hdf[0].cylinders) * hdf[0].heads * hdf[0].sectors) >> 11));
			BootPrint(s);
			sprintf(s, "Offset: %ld", hdf[0].offset);
			BootPrint(s);
		//		printf("c3: %x\n",CheckSum());
		}
		if(OpenHardfile(1))
		{
			switch(hdf[1].type)
			{
				case (HDF_FILE | HDF_SYNTHRDB):
					sprintf(s, "\nHardfile 1 (with fake RDB): %.8s.%.3s", hdf[1].file.name, &hdf[1].file.name[8]);
					break;
				case HDF_FILE:
					sprintf(s, "\nHardfile 1: %.8s.%.3s", hdf[1].file.name, &hdf[1].file.name[8]);
					break;
				case HDF_CARD:
					sprintf(s, "\nHardfile 1: using entire SD card");
					break;
				default:
					sprintf(s, "\nHardfile 1: using SD card partition %d",hdf[1].type-HDF_CARD);	// Number from 1
					break;
			}
			BootPrint(s);
			sprintf(s, "CHS: %u.%u.%u", hdf[1].cylinders, hdf[1].heads, hdf[1].sectors);
			BootPrint(s);
			sprintf(s, "Size: %lu MB", ((((unsigned long) hdf[1].cylinders) * hdf[1].heads * hdf[1].sectors) >> 11));
			BootPrint(s);
			sprintf(s, "Offset: %ld", hdf[1].offset);
			BootPrint(s);
		}
		ConfigIDE(config.enable_ide, config.hardfile[0].present && config.hardfile[0].enabled,
			config.hardfile[1].present && config.hardfile[1].enabled);
	}

    ConfigCPU(config.cpu);
    ConfigMemory(config.memory);
    ConfigChipset(config.chipset);
    ConfigFloppy(config.floppy.drives, config.floppy.speed);
    ConfigVideo(config.filter.hires, config.filter.lores, config.scanlines);
    ConfigMisc(config.misc);

    if(reloadkickstart) {
//		WaitTimer(100);
		EnableOsd();
		SPI(OSD_CMD_RST);
		rstval |= (SPI_RST_CPU | SPI_CPU_HLT);
		SPI(rstval);
		DisableOsd();
		SPIN; SPIN; SPIN; SPIN;
		UploadActionReplay();
		if (romsize=UploadKickstart(config.kickdir,config.kickstart.name))
		{
			result=1;
		}
		else
		{
			strcpy(config.kickstart.name, "KICK    ");
			result=UploadKickstart(config.kickdir,config.kickstart.name);
		}
		if(result && romsize<1024)
		{
			/* Attempt to upload an extended ROM, but only if the kickstart file is less than a megabyte */
			if(!UploadExtROM(config.extromdir,config.extrom.name))
			{
				strcpy(config.extrom.name, "EXTENDED");
				result=UploadExtROM(config.extromdir,config.extrom.name);
			}
			ClearError(ERROR_FILESYSTEM); /* We don't need to report a missing ExtROM yet */
		}
    }
	return(result);
}


unsigned char SaveConfiguration(char *filename)
{
	if(!filename)
		filename=configfilename;	// Use slot-based filename if none provided.

    // save configuration data
	ChangeDirectory(0); // Config files always live in the root directory
    if (FileOpen(&file, filename))
    {
        if (file.size != sizeof(config))
        {
            file.size = sizeof(config);
            if (!UpdateEntry(&file))
                return(0);
        }

        memset((void*)&sector_buffer, 0, sizeof(sector_buffer));
        memcpy((void*)&sector_buffer, (void*)&config, sizeof(config));
        FileWrite(&file, sector_buffer);
        return(1);
    }
    else
    {
		ClearError(ERROR_FILESYSTEM);
        printf("Configuration file not found!\r");
        printf("Trying to create a new one...\r");
        strncpy(file.name, filename, 11);
        file.attributes = 0;
        file.size = sizeof(config);
        printf("Config size is %x (%x) - address is %x\n",sizeof(config),file.size,&config);
        if (FileCreate(0, &file))
        {
            printf("File created.\r");
            printf("Trying to write new data...\r");
            memset((void*)sector_buffer, 0, sizeof(sector_buffer));
            memcpy((void*)sector_buffer, (void*)&config, sizeof(config));

            if (FileWrite(&file, sector_buffer))
            {
                printf("File written successfully.\r");
                return(1);
            }
            else
                printf("File write failed!\r");
        }
        else
            printf("File creation failed!\r");
    }
    return(0);
}

