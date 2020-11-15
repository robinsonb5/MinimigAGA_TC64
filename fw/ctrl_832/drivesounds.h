#ifndef DRIVESOUNDS_H
#define DRIVESOUNDS_H

enum DriveSound_Type {
	DRIVESOUND_INSERT=0,DRIVESOUND_EJECT,DRIVESOUND_MOTORSTART,DRIVESOUND_MOTORLOOP,DRIVESOUND_MOTORSTOP,
	DRIVESOUND_STEP,DRIVESOUND_STEP2,DRIVESOUND_STEP3,DRIVESOUND_STEP4,
	DRIVESOUND_HDDSTEP,DRIVESOUND_HDDSTEP2,DRIVESOUND_HDDSTEP3,DRIVESOUND_HDDSTEP4
};
#define DRIVESOUND_COUNT (DRIVESOUND_HDDSTEP4+1)

#define DRIVESOUNDS_FLOPPY 1
#define DRIVESOUNDS_HDD 2

void drivesounds_enable();
void drivesounds_disable();
void drivesounds_on();
void drivesounds_off();
void drivesounds_queueevent(enum DriveSound_Type type);
void drivesounds_stop();
void drivesounds_start();
int drivesounds_fill();
int drivesounds_init(const char *filename);
int drivesounds_loaded();

#endif

