#include <stdio.h>

enum DriveSound_Type {
	DRIVESOUND_INSERT=0,DRIVESOUND_EJECT,DRIVESOUND_MOTORSTART,DRIVESOUND_MOTORLOOP,DRIVESOUND_MOTORSTOP,
	DRIVESOUND_STEP1,DRIVESOUND_STEP2,DRIVESOUND_STEP3,DRIVESOUND_STEP4,DRIVESOUND_HDDSTEP
};

int gains[]=
{
	90,90,90,90,90,220,220,220,220,30
};


void emit_longword_be(unsigned int v)
{
	putchar((v>>24)&255);
	putchar((v>>16)&255);
	putchar((v>>8)&255);
	putchar(v&255);
}

unsigned char buffer[512];

int bufferswap(unsigned char *b,int c,int gain)
{
	while(c>0)
	{
		int a=b[0] | (b[1]<<8);
		if(a&0x8000)
			a=-(0x10000-a);
		a=(a*gain)>>8;
		b[0]=a>>8;
		b[1]=a&0xff;
		b+=2;
		c-=2;
	}
}


int main(int argc,char **argv)
{
	int i;
	printf("DRIVESND");
	for(i=1;i<argc;++i)
	{
		FILE *f;
		unsigned int l;
		fprintf(stderr,"File %s\n",argv[i]);
		f=fopen(argv[i],"rb");
		fseek(f,0,SEEK_END);
		l=ftell(f);
		fseek(f,0,SEEK_SET);
		fprintf(stderr,"Size %d\n",l);
		emit_longword_be(i-1);
		emit_longword_be(l);
		fprintf(stderr,"Gain: %d\n",gains[i-1]);
		while(l>0)
		{
			int r=fread(buffer,1,512,f);
			bufferswap(buffer,r,gains[i-1]);
			l-=r;
			fwrite(buffer,1,r,stdout);
		}
	}
	emit_longword_be(0);
	emit_longword_be(0);
	return(0);
}

