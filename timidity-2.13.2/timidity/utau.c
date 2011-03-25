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

extern char* utau;//the utauloid voicebank path
Riff* default_voice=0;

SpecialPatch* default_utau_patch=0;


void karaoke_write_wave(short* samples,int length,int sample_rate,char* filename)
{

	
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

Riff * utau_read_riff(char* filename)
{

	Riff* ret=0;
	printf("reading %s\n",filename);
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
	size*=2;
	size=100696064;
	size*=2;
	printf("size = %i\n",size);
	ret->samples=(short*)malloc(size);
	memset(ret->samples,0,size);
	l=fread(ret->samples,1,header.data_chunk_size,f);
	
	ret->sample_rate=header.sample_rate;
	ret->length=size;
	//if(l+0x1000!=ret->length)  {printf("inVALID data length\n");exit(1);}
	//requires page size	

	//header.compression=1;
	//header.number_of_channels=1;
	//header.sample_rate=sample_rate;
	//header.block_align=4;
	//header.bytes_per_sec=header.sample_rate * header.block_align;
	//header.bits_per_sample=16;

	
	fclose(f);
	return ret;
}

void utau_init()
{
	
	default_voice=utau_read_riff(utau);
	if(default_voice)
	{ 
		//add one zero sample for resampling;
		printf("default voice loaded\n");
		default_utau_patch=(SpecialPatch *)safe_malloc(sizeof(SpecialPatch));
		default_utau_patch->samples=1;
		Sample* s=(Sample *)safe_malloc(sizeof(Sample));
		memset(s,0,sizeof(Sample));
		default_utau_patch->sample=s;
		int len=default_voice->length;
		karaoke_write_wave(default_voice->samples,default_voice->length,default_voice->sample_rate,"/tmp/input.wav");		

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
	else
	{
		printf("error: invalid voicebank\n");
		return 0;
	}
	
	
}

#ifdef LOOKUP_HACK
#error utau does not work with lookup hack
#endif


void utau_hack_sample(Sample* s)//rename utau_dump_sample
{
	int i;
	int sz;
	#define LOG(v) if(s->v!=0) printf("s->%s= %i;\n",#v,(int)s->v);
	#define LOG6(v) for(i=0;i<6;i++) if(s->v[i]!=0) printf("s->%s[%i]= %i;\n",#v,i,(int)s->v[i]);
	#define LOGF(v) if(s->v!=0) printf("s->%s= %f;\n",#v,s->v);
	printf("<---\n");
//==================================
	LOG(loop_start);
	LOG(loop_end);
	LOG(data_length);
//-----------------------------------
	LOG(data);
//-------------------------------
	LOG(sample_rate);
	LOG(low_freq);
	LOG(high_freq);
	LOG(root_freq);
//-------------------------------
	LOG(panning);
	LOG(note_to_use);
//----------------------------
	LOG6(envelope_rate);
	LOG6(envelope_offset);
	LOG6(modenv_rate)
	LOG6(modenv_offset);
//----------------------------------------
	LOG(volume);
//----------------------------------------
	LOG(tremolo_sweep_increment);
	LOG(tremolo_phase_increment);
	LOG(vibrato_sweep_increment)
	LOG(vibrato_control_ratio);
//-------------------------------------------
	LOG(tremolo_depth);
	LOG(vibrato_depth);
//-------------------------------------------
	LOG(modes);
	LOG(data_alloced);
	LOG(low_vel);
	LOG(high_vel);
//-------------------------------------------
	LOG(cutoff_freq);
	LOG(resonance);
//-------------------------------------------
	LOG(tremolo_to_pitch);
	LOG(tremolo_to_fc)
	LOG(modenv_to_pitch)
	LOG(modenv_to_fc);
//---
LOG6(envelope_keyf);
LOG6(envelope_velf);
LOG6(modenv_keyf);
LOG6(modenv_velf);
//---
	LOG(vel_to_fc);
	LOG(key_to_fc);
//--------------------------------
	LOG(vel_to_resonance);
//--------------------------------
	LOG(envelope_velf_bpo);
	LOG(modenv_velf_bpo);
	LOG(key_to_fc_bpo);
	LOG(vel_to_fc_threshold);
//--------------------------------
	LOG(vibrato_delay);
	LOG(tremolo_delay);
	LOG(envelope_delay);
	LOG(modenv_delay);
//--------------------------------
	LOG(scale_freq);
	LOG(scale_factor);
//--------------------------------
	LOG(inst_type);
//--------------------------------
	LOG(sf_sample_index);
	LOG(sf_sample_link);
//--------------------------------
	LOG(sample_type);
	LOGF(root_freq_detected);
	LOG(transpose_detected);
	LOG(chord);
	printf("--->\n");
	
}

SpecialPatch* utau_special_patch()
{
	return default_utau_patch;
}
