/*--------------------------------------------------------------------*/
/* ATR2CAR                                                            */
/* by GienekP                                                         */
/* (c) 2022                                                           */
/*--------------------------------------------------------------------*/
#include <stdio.h>
/*--------------------------------------------------------------------*/
typedef unsigned char U8;
/*--------------------------------------------------------------------*/
#define RAMPROC 0x0100
/*--------------------------------------------------------------------*/
#define FLASHAUTO 0
#define FLASH128K 1
#define FLASH256K 2
#define FLASH512K 3
#define SEC128 1
#define SEC256 2
#define OLDADDR 0
#define NEWADDR 1
/*--------------------------------------------------------------------*/
#define CAR128 (128*1024)
#define CAR256 (256*1024)
#define CAR512 (512*1024)
#define CARMAX CAR512
#define LDRSIZE (8*1024)
#define ATRMAX (CARMAX-LDRSIZE-128*5)
/*--------------------------------------------------------------------*/
#include "starter128.h"
#include "starter256.h"
/*--------------------------------------------------------------------*/
void checkATR(const U8 *data, U8 *sector, unsigned int *type)
{
	if ((data[0]==0x96) && (data[1]==02))
	{
		if ((data[4]==0x80) && (data[5]==0x00))
		{
			*sector=SEC128;
		}
		else if ((data[4]==0x00) && (data[5]==0x01))
		{
			*sector=SEC256;
			if (data[2]&0x0F)
			{
				*type=3*128;
			}
			else
			{
				*type=6*128;
			};
		};
	};
}
/*--------------------------------------------------------------------*/
unsigned int loadATR(const char *filename, U8 *data, U8 *sector, unsigned int *type)
{
	U8 header[16];
	unsigned int ret=0;
	int i;
	FILE *pf;
	for (i=0; i<ATRMAX; i++)
	{
		data[i]=0xFF;
	};
	pf=fopen(filename,"rb");
	if (pf)
	{
		i=fread(header,sizeof(U8),16,pf);
		if (i==16)
		{
			checkATR(header,sector,type);
			if ((*sector==SEC128) || (*sector==SEC256)) 
			{
				ret=fread(data,sizeof(U8),ATRMAX,pf);
			}
			else
			{
				printf("Wrong sector header.\n");
			};
		}
		else
		{
			printf("Wrong ATR header size.\n");
		}
		fclose(pf);
	}
	else
	{
		printf("\"%s\" does not exist.\n",filename);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int assignFlash(unsigned int atrsize, U8 *flash)
{
	unsigned int ret;
	if (*flash==FLASHAUTO)
	{
		if (atrsize<=(CAR128-LDRSIZE-5*128)) {*flash=FLASH128K;}
		else if (atrsize>(CAR256-LDRSIZE-5*128)) {*flash=FLASH512K;}
		else {*flash=FLASH256K;}
		
	}
	else
	{
		printf("Forced ");
	};
	switch (*flash)
	{
		case FLASH128K: {printf("Flash: 128kB\n");ret=CAR128;} break;
		case FLASH256K: {printf("Flash: 256kB\n");ret=CAR256;} break;
		case FLASH512K: {printf("Flash: 512kB\n");ret=CAR512;} break;
		default: {printf("Flash AUTO\n");ret=0;} break;	
	};
	return ret;
}
/*--------------------------------------------------------------------*/
void replace(U8 mode, U8 *atrdata, unsigned int i)
{
	static U8 f=0;
	if (mode==NEWADDR)
	{
		atrdata[i+1]=((RAMPROC)&0xFF);
		atrdata[i+2]=(((RAMPROC)>>8)&0xFF);
	};
	if (f==0)
	{
		f=1;
		if (mode==NEWADDR)
		{
			printf("Replace calls:\n");
		}
		else
		{
			printf("Possible calls:\n");
		};
	};
}
/*--------------------------------------------------------------------*/
void checkXINT(U8 *atrdata, U8 mode)
{
	unsigned int i;
	for (i=0; i<ATRMAX-3; i++)
	{
		if ((atrdata[i+1]==0x53) && (atrdata[i+2]==0xE4))
		{
			if (atrdata[i]==0x20)
			{
				replace(mode,atrdata,i);
				printf(" JSR JDSKINT ; 0x%06X 20 53 E4 -> 20 00 01\n",i+16);
				
			};
			if (atrdata[i]==0x4C)
			{
				replace(mode,atrdata,i);
				printf(" JMP JDSKINT ; 0x%06X 4C 53 E4 -> 4C 00 01\n",i+16);
			};			
		};
		if ((atrdata[i+1]==0xB3) && (atrdata[i+2]==0xC6))
		{
			if (atrdata[i]==0x20)
			{
				replace(mode,atrdata,i);
				printf(" JSR DSKINT ; 0x%06X 20 B3 C6 -> 20 00 01\n",i+16);
			};
			if (atrdata[i]==0x4C)
			{
				replace(mode,atrdata,i);
				printf(" JMP DSKINT ; 0x%06X 4C B3 C6 -> 4C 00 01\n",i+16);
			};			
		};
		if ((atrdata[i+1]==0x59) && (atrdata[i+2]==0xE4))
		{
			if (atrdata[i]==0x20)
			{
				replace(mode,atrdata,i);
				printf(" JSR JSIOINT ; 0x%06X 20 59 E4 -> 20 00 01\n",i+16);
			};
			if (atrdata[i]==0x4C)
			{
				replace(mode,atrdata,i);
				printf(" JMP JSIOINT ; 0x%06X 4C 59 E4 -> 4C 00 01\n",i+16);
			};			
		};
		if ((atrdata[i+1]==0x33) && (atrdata[i+2]==0xC9))
		{
			if (atrdata[i]==0x20)
			{
				replace(mode,atrdata,i);
				printf(" JSR SIOINT ; 0x%06X 20 33 C9 -> 20 00 01\n",i+16);
			};
			if (atrdata[i]==0x4C)
			{
				replace(mode,atrdata,i);
				printf(" JMP SIOINT ; 0x%06X 4C 3 C9 -> 4C 00 01\n",i+16);
			};			
		};	
	};
}
/*--------------------------------------------------------------------*/
unsigned int buildCar128(const U8 *loader, unsigned int stsize, 
                         const U8 *atrdata, unsigned int atrsize, 
                         U8 *cardata, unsigned int carsize)
{
	unsigned int i,sum=0,toend=stsize;
	for (i=0; i<CARMAX; i++)
	{
		cardata[i]=0xFF;
	};
	if (atrsize>(carsize-LDRSIZE-128))
	{
		atrsize=(carsize-LDRSIZE-128);
	};
	for (i=0; i<atrsize; i++)
	{
		cardata[i+128]=atrdata[i];
	};
	if (toend>LDRSIZE)
	{
		toend=LDRSIZE;
	};
	for (i=0; i<toend; i++)
	{
		cardata[carsize-toend+i]=loader[stsize-toend+i];
	};
	for (i=0; i<carsize; i++)
	{
		sum+=cardata[i];
	};
	return sum;
}
/*--------------------------------------------------------------------*/
unsigned int buildCar256(const U8 *loader, unsigned int stsize, 
                         const U8 *atrdata, unsigned int atrsize, 
                         U8 *cardata, unsigned int carsize, unsigned int type)
{
	unsigned int i,j,sum=0,toend=stsize;
	for (i=0; i<CARMAX; i++)
	{
		cardata[i]=0xFF;
	};
	
	if (atrsize>(carsize-LDRSIZE-5*128))
	{
		atrsize=(carsize-LDRSIZE-5*128);
	};
	
	for (j=1; j<4; j++)
	{
		for (i=0; i<128; i++)
		{
			cardata[j*256+i]=atrdata[(j-1)*128+i];
		};
	};
	for (i=(type); i<atrsize; i++)
	{
		cardata[i+(4*256)-(type)]=atrdata[i];
	};
	if (toend>LDRSIZE)
	{
		toend=LDRSIZE;
	};
	for (i=0; i<toend; i++)
	{
		cardata[carsize-toend+i]=loader[stsize-toend+i];
	};
	for (i=0; i<carsize; i++)
	{
		sum+=cardata[i];
	};
	return sum;
}
/*--------------------------------------------------------------------*/
U8 saveCAR(const char *filename, U8 *data, unsigned int sum, U8 flash)
{
	U8 header[16]={0x43, 0x41, 0x52, 0x54, 0x00, 0x00, 0x00, 0xFF,
		           0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
	U8 ret=0;
	int i,carsize;
	FILE *pf;
	switch (flash)
	{
		case FLASH128K: {header[7]=0x23;carsize=CAR128;} break;
		case FLASH256K: {header[7]=0x24;carsize=CAR256;} break;
		case FLASH512K: {header[7]=0x25;carsize=CAR512;} break;
		default: {header[7]=0x00;carsize=0;} break;
	};
	header[8]=((sum>>24)&0xFF);
	header[9]=((sum>>16)&0xFF);
	header[10]=((sum>>8)&0xFF);
	header[11]=(sum&0xFF);
	pf=fopen(filename,"wb");
	if (pf)
	{
		i=fwrite(header,sizeof(U8),16,pf);
		if (i==16)
		{
			i=fwrite(data,sizeof(U8),carsize,pf);
			if (i==carsize)
			{
				ret=1;
			};			
		};
		fclose(pf);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
void atr2car(const char *atrfn, const char *carfn, U8 m, U8 f)
{
	U8 atrdata[ATRMAX];
	U8 cardata[CARMAX];
	U8 mode=m;
	U8 flash=f;
	U8 sector;
	unsigned int atrsize;
	unsigned int carsize;
	unsigned int sum;
	unsigned int type;
	atrsize=loadATR(atrfn,atrdata,&sector,&type);
	if (atrsize)
	{
		printf("Load \"%s\"\n",atrfn);
		printf("ATR size: %i\n",atrsize+16);	
		carsize=assignFlash(atrsize,&flash);
		if (((sector==SEC128) || (sector==SEC256)))
		{
			checkXINT(atrdata,mode);
			if (sector==SEC128) 
			{
				printf("Sector: 128\n");
				sum=buildCar128(starter128_bin,starter128_bin_len,atrdata,atrsize,cardata,carsize);
			}
			else
			{
				printf("Sector: 256\n");
				sum=buildCar256(starter256_bin,starter256_bin_len,atrdata,atrsize,cardata,carsize,type);
			};
			if (saveCAR(carfn,cardata,sum,flash))
			{
				printf("Save \"%s\"\n",carfn);
			}
			else
			{
				printf("Save \"%s\" ERROR!\n",carfn);
			};
		}
		else
		{
			printf("Sector: unsuported\n");
		};
	}
	else
	{
		printf("Load \"%s\" ERROR!\n",atrfn);
	};
}
/*--------------------------------------------------------------------*/
void flashSize(const char *str, U8 *flash)
{
	if ((str[0]=='-') && (str[1]=='1') && (str[2]=='2') && (str[3]=='8'))
	{
		*flash=FLASH128K;
	} else
	if ((str[0]=='-') && (str[1]=='2') && (str[2]=='5') && (str[3]=='6'))
	{
		*flash=FLASH256K;
	} else
	if ((str[0]=='-') && (str[1]=='5') && (str[2]=='1') && (str[3]=='2'))
	{
		*flash=FLASH512K;
	};	
}
/*--------------------------------------------------------------------*/
void modeSel(const char *str, U8 *mode)
{
	if ((str[0]=='-') && ((str[1]=='c') || (str[1]=='C')))
	{
		*mode=NEWADDR;
	};
}
/*--------------------------------------------------------------------*/
int main( int argc, char* argv[] )
{	
	U8 mode=OLDADDR;
	U8 flash=FLASHAUTO;
	printf("ATR2CAR - ver: %s\n",__DATE__);
	if (argc==3)
	{
		atr2car(argv[1],argv[2],mode,flash);
	}
	else if (argc==4)
	{
		modeSel(argv[3],&mode);
		flashSize(argv[3],&flash);
		atr2car(argv[1],argv[2],mode,flash);
	}
	else if (argc==5)
	{
		modeSel(argv[3],&mode);
		flashSize(argv[3],&flash);
		modeSel(argv[4],&mode);
		flashSize(argv[4],&flash);
		atr2car(argv[1],argv[2],mode,flash);
	} else
	{
		printf("(c) GienekP\n");
		printf("use:\natr2car file.atr file.car [-c] [-128|-256|-512]\n");
	};
	printf("\n");
	return 0;
}
/*--------------------------------------------------------------------*/
