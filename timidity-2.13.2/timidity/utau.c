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
	ret->samples=(short*)malloc(header.data_chunk_size);
	l=fread(ret->samples,1,header.data_chunk_size,f);
	ret->sample_rate=header.sample_rate;
	ret->length=header.data_chunk_size;
	if(l!=ret->length)  {printf("inVALID data length\n");exit(1);}	

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

		printf("default voice loaded\n");
		default_utau_patch=(SpecialPatch *)safe_malloc(sizeof(SpecialPatch));
		default_utau_patch->samples=1;
		Sample* s=(Sample *)safe_malloc(sizeof(Sample));
		memset(s,0,sizeof(Sample));
		default_utau_patch->sample=s;
		int len=default_voice->length;
//some magic numbers taken from gunshot
s->loop_start= 152555520;
s->loop_end= 152559616;
s->data_length= 152559616;
s->data= default_voice->samples;
s->sample_rate= 44101;
s->low_freq= 8176;
s->high_freq= 12543854;
s->root_freq= 261626;
s->panning= 63;
s->envelope_rate[0]= 1073741824;
s->envelope_rate[1]= 16;
s->envelope_rate[2]= 822879;
s->envelope_rate[3]= 1071290;
s->envelope_rate[4]= 1071290;
s->envelope_rate[5]= 1071290;
s->envelope_offset[0]= 1073725440;
s->envelope_offset[1]= 1073709056;
s->envelope_offset[2]= 42942464;
s->modenv_rate[0]= 1073741824;
s->modenv_rate[1]= 1073741824;
s->modenv_rate[2]= 1073741824;
s->modenv_rate[3]= 1073741824;
s->modenv_rate[4]= 1073741824;
s->modenv_rate[5]= 1073741824;
s->modenv_offset[0]= 1073725440;
s->modenv_offset[1]= 1073709056;
s->modenv_offset[2]= 1073692672;
s->volume= 1;
s->modes= 65;
s->data_alloced= 1;
s->high_vel= 127;
s->vel_to_fc= -2400;
s->envelope_velf_bpo= 64;
s->modenv_velf_bpo= 64;
s->key_to_fc_bpo= 60;
s->vel_to_fc_threshold= 64;
s->scale_freq= 60;
s->scale_factor= 410;
s->inst_type= 1;
s->sf_sample_index= 189;
s->sf_sample_link= -1;
s->sample_type= 1;



		
	}
	else
	{
		printf("error: invalid voicebank\n");
		return 0;
	}
	
	
}

#ifdef LOOKUP_HACK
#error utau does not work with lookup hack
typedef struct _Sample {
  splen_t
    loop_start, loop_end, data_length;
  int32
    sample_rate, low_freq, high_freq, root_freq;
  int8 panning, note_to_use;
  int32
    envelope_rate[6], envelope_offset[6],
	modenv_rate[6], modenv_offset[6];
  FLOAT_T
    volume;
  sample_t
    *data;
  int32
    tremolo_sweep_increment, tremolo_phase_increment,
    vibrato_sweep_increment, vibrato_control_ratio;
  int16
    tremolo_depth;
  int16 vibrato_depth;
  uint8
    modes, data_alloced,
    low_vel, high_vel;
  int32 cutoff_freq;	/* in Hz, [1, 20000] */
  int16 resonance;	/* in centibels, [0, 960] */
  /* in cents, [-12000, 12000] */
  int16 tremolo_to_pitch, tremolo_to_fc, modenv_to_pitch, modenv_to_fc,
	  envelope_keyf[6], envelope_velf[6], modenv_keyf[6], modenv_velf[6],
	  vel_to_fc, key_to_fc;
  int16 vel_to_resonance;	/* in centibels, [-960, 960] */
  int8 envelope_velf_bpo, modenv_velf_bpo,
	  key_to_fc_bpo, vel_to_fc_threshold;	/* in notes */
  int32 vibrato_delay, tremolo_delay, envelope_delay, modenv_delay;	/* in samples */
  int16 scale_freq;	/* in notes */
  int16 scale_factor;	/* in 1024divs/key */
  int8 inst_type;
  int32 sf_sample_index, sf_sample_link;	/* for stereo SoundFont */
  uint16 sample_type;	/* 1 = Mono, 2 = Right, 4 = Left, 8 = Linked, $8000 = ROM */
  FLOAT_T root_freq_detected;	/* root freq from pitch detection */
  int transpose_detected;	/* note offset from detected root */
  int chord;			/* type of chord for detected pitch */
} Sample;
#endif


void utau_hack_sample(Sample* s)
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
