#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#ifdef __W32__
#include "interface.h"
#endif
#include <stdio.h>
#include <stdlib.h>

#ifndef NO_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <math.h>
#ifdef __W32__
#include <windows.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include "timidity.h"
#include "common.h"
#include "instrum.h"
#include "playmidi.h"
#include "readmidi.h"
#include "output.h"
#include "mix.h"
#include "controls.h"
#include "miditrace.h"
#include "recache.h"
#include "arc.h"
#include "reverb.h"
#include "wrd.h"
#include "aq.h"
#include "freq.h"
// AA 2E = '+' or '↑'

#ifdef LOOKUP_HACK
#error utau does not work with lookup hack
#endif

static char* utau_text="no lyrics";


struct riff_header
{	
		int riff;
		int chunk_data_size;
		int wave;
		int fmt_chunk_id;
		int fmt_chunk_data_size;
		short compression;
		short number_of_channels;
		int sample_rate;
		int bytes_per_sec;
		short block_align;
		short bits_per_sample;
		int data_chunk;
		int data_chunk_size;
};

typedef struct _Riff
{
	short* samples;
	int length;
	int sample_rate;
} Riff;

typedef struct _Oto
{
	char name[64];
	char alias[64];
	int offset,consonant,cutoff,pre_utterance,overlap;
} Oto;

Oto voicebank[4000];

extern char* utau;//the utauloid voicebank path




static void utau_write_wave(short* samples,int length,int sample_rate,char* filename) //only for debugging
{
	unlink(filename);
	struct riff_header header;
	header.riff='FFIR';
	header.chunk_data_size=sizeof(struct riff_header)-8+length;
	header.wave='EVAW';
	header.fmt_chunk_id=' tmf';
	header.fmt_chunk_data_size=16;
	header.compression=1;
	header.number_of_channels=1;
	header.sample_rate=sample_rate;
	header.block_align=4;
	header.bytes_per_sec=header.sample_rate * header.block_align;
	header.bits_per_sample=16;
	header.data_chunk='atad';
	header.data_chunk_size=length;
	FILE* f=fopen(filename,"w");
	fwrite(&header,1,sizeof(header),f);
	fwrite(samples,1,length,f);
	fclose(f);
}

#if 0
void utau_init_sample(Sample* s)
{
//must be page aligned
s->sample_rate= 44100;
s->low_freq= 8176;
s->high_freq= 1975533;
s->data=default_voice->samples;
//need very long buffer 

s->data_length=default_voice->length;
s->loop_start=default_voice->length-0x1000;
s->loop_end = default_voice->length;

//s->loop_start= 98365440;
//s->loop_end= 100560896;
//s->data_length= 100696064;

s->root_freq= 492882/2;
printf("root frq in hz: %i",s->root_freq/100);
s->panning= 63;
//#need big numbers,required
s->envelope_rate[0]= 178549877;
s->envelope_rate[1]= 50;
s->envelope_rate[2]= 872;
s->envelope_rate[3]= 3570229;
s->envelope_rate[4]= 3570229;
s->envelope_rate[5]= 3570229;
s->envelope_offset[0]= 1073725440;
s->envelope_offset[1]= 1073709056;
s->envelope_offset[2]= 1048985600;
s->modenv_rate[0]= 119030157;
s->modenv_rate[1]= 1073741824;
s->modenv_rate[2]= 419180;
s->modenv_rate[3]= 2142581;
s->modenv_rate[4]= 2142581;
s->modenv_rate[5]= 2142581;
s->modenv_offset[0]= 1073725440;
s->modenv_offset[1]= 1073709056;
s->modenv_offset[2]= 107364352;

s->volume= 20;
s->modes= 65;
s->data_alloced= 1;
#if 0
s->high_vel= 127;
s->cutoff_freq= 5371;//ignored
//s->resonance= 110;
s->modenv_to_pitch= -113;
s->modenv_to_fc= 1800;
s->vel_to_fc= -2400;
s->envelope_velf_bpo= 64;
s->modenv_velf_bpo= 64;
s->key_to_fc_bpo= 60;
s->vel_to_fc_threshold= 64;
#endif
s->scale_freq= 70;
s->scale_factor= 1024;
s->inst_type= 1;
#if 0
s->sf_sample_index= 561;
s->sf_sample_link= -1;
#endif
s->sample_type= 1;
//pre_resample(s);
s->chord = -1;
s->root_freq_detected = freq_fourier(s, &(s->chord));
printf("freq: %f %i\n",s->root_freq_detected,s->chord);
if(s->chord==-1) s->root_freq=s->root_freq_detected*1000;
}
#endif

Riff * utau_read_riff(char* filename)
{

	Riff* ret=0;
	FILE* f=fopen(filename,"r");
	if(f==NULL) return 0;
	ret=malloc(sizeof(Riff));
	struct riff_header header;
	int l=fread(&header,1,sizeof(header),f);
	if(l!=sizeof(header))  {printf("inVALID WAVE HEADER size\n");exit(1);}
	if(!(header.riff=='FFIR' && header.wave=='EVAW')) {printf("inVALID WAVE HEADER\n");exit(1);}
	if(!(header.fmt_chunk_id==' tmf' && header.fmt_chunk_data_size==16)) {printf("inVALID format\n");exit(1);}
	if(!(header.data_chunk=='atad')) {printf("inVALID data\n");exit(1);}
	int size=header.data_chunk_size+0x1000;
	if(size % 0x1000 !=0) size += (0x1000 - (size % 0x1000));
	ret->samples=(short*)malloc(size);
	memset(ret->samples,0,size);
	l=fread(ret->samples,1,header.data_chunk_size,f);
	
	ret->sample_rate=header.sample_rate;
	ret->length=size;
	//if(l+0x1000!=ret->length)  {printf("inVALID data length\n");exit(1);}
	//header.number_of_channels=1;
	//header.sample_rate=sample_rate;
	//header.block_align=4;
	//header.bytes_per_sec=header.sample_rate * header.block_align;
	//header.bits_per_sample=16;
	//TODO check input
	fclose(f);
	return ret;
}

static void parse_oto(char* oto,Oto* o)
{
	int i;
	for(i=0;i<1024;i++) if (oto[i]=='\r'|| oto[i]=='\n') oto[i]=0;
	for(i=0;i<1024;i++) if (oto[i]=='='|| oto[i]==',') oto[i]=' ';
	sscanf(oto,"%s %s %i %i %i %i %i",&o->name,&o->alias,&o->consonant,&o->offset,&o->cutoff,&o->pre_utterance,&o->overlap);
}

static int oto_compare_name (const void * a, const void * b)
{
  Oto* c=(Oto*)a;
  Oto* d=(Oto*)b;
  return strcmp(c->name,d->name);
}

static int oto_compare_alias (const void * a, const void * b)
{
  Oto* c=(Oto*)a;
  Oto* d=(Oto*)b;
  return strcmp(c->alias,d->alias);
}


void utau_init()
{
	char oto_ini[1024];
	sprintf(oto_ini,"%s/oto.ini",utau);
	FILE* f=fopen(oto_ini,"r");
	if(f==0) {printf("could not find oto ini\n");exit(0);}
	char oto[1024];
	int i=0;

	while(fgets(oto,sizeof(oto),f))
	{
	parse_oto(oto,&voicebank[i]);
	i++;
	}
	//sort (values, 6, sizeof(int), compare);
	qsort(voicebank,i,sizeof(Oto),oto_compare_name);
	int count=i;	
	for(i=0;i<count;i++)
	{
	char wav_file[1024];
	sprintf(wav_file,"%s/%s",utau,voicebank[i].name);
	printf("FIXME: load wave file %s %x and link oto %i %i\n",wav_file,utau_read_riff(wav_file),i,i-1);
	//TODO: oto_load(i,i-1);
	}
}

void utau_set_text(char* text)
{
	//printf("set text: %s\n",text);
	utau_text=text;
	//use bsearch to find a patch
}

SpecialPatch* utau_special_patch()
{
	return NULL;
}

char* utau_get_text()
{
	return utau_text;
}
