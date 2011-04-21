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

static char* utau_text="o";


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
	Riff* wavefile;
	Sample* sample;
	int samples;
} Oto;

Oto voicebank[4000];
int voicebank_count;

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
static int32 to_offset(int32 offset)
{
	return offset << 14;
}
#endif

static int32 env_offset(int offset)
{
    return (int32)offset << (7+15);
}

/* calculate ramp rate in fractional unit;
 * diff = 8bit, time = msec
 */
static int32 env_rate(int diff, double msec)
{
    double rate;

    if(msec < 6)
	msec = 6;
    if(diff == 0)
	diff = 255;
    diff <<= (7+15);
    rate = ((double)diff / play_mode->rate) * control_ratio * 1000.0 / msec;
    if(fast_decay)
	rate *= 2;
    return (int32)rate;
}





void utau_init_sample(int i)
{
Sample* s=voicebank[i].sample;
Riff* riff=voicebank[i].wavefile;
int offset=voicebank[i].offset;
int consonant=voicebank[i].offset;
int cutoff=voicebank[i].cutoff;
int pre_utterance=voicebank[i].pre_utterance;
int overlap=voicebank[i].overlap;
int length = (riff->length/2);
s->sample_rate=riff->sample_rate;

if(pre_utterance > 0 || overlap >0 )
printf("FIXME: %s: pre utterance: %i  overlap: %i\n",voicebank[i].name,pre_utterance,overlap);

//offset+=consonant;
offset*=riff->sample_rate;
offset/=1000;
cutoff*=riff->sample_rate;
cutoff/=1000;

offset=0;//ignore offset
cutoff=0;


s->data_length =  (length-offset-cutoff) << FRACTION_BITS;
s->loop_end    =  (length-offset-cutoff) << FRACTION_BITS;
s->loop_start  = s->loop_end - 0x1000;
s->data=riff->samples+offset;

s->panning= 63;
s->volume= 20;
s->modes= 3;//FIXME use envelope
s->data_alloced= 1;

s->scale_freq= 70;
s->scale_factor= 1024;
s->inst_type= 1;
s->sample_type= 1;
s->chord = -1;
s->root_freq_detected = freq_fourier(s, &(s->chord));
s->root_freq=s->root_freq_detected*1000;
s->low_freq= s->root_freq/1.4;
s->high_freq= s->root_freq*1.4;

//printf("root frq in hz: %f\n",((float)s->root_freq)/1000);



	/* envelope (0,1:attack, 2:sustain, 3,4,5:release) */
#if 1
	s->modes |= MODES_ENVELOPE;
	int rate1=env_offset(255);
	int rate2=env_offset(255);
	int rate3=env_offset(200);
	/* attack */
	s->envelope_offset[0] = env_offset(255);
	s->envelope_rate[0]   = rate1;	
	s->envelope_offset[1] = s->envelope_offset[0];
	s->envelope_rate[1]   = 0; /* skip this stage */
	/* sustain */
	s->envelope_offset[2] = s->envelope_offset[1];
	s->envelope_rate[2]   = rate3;//do not skip sustain
	/* release */
	s->envelope_offset[3] = env_offset(0);
	s->envelope_rate[3]   = rate2;
	s->envelope_offset[4] = s->envelope_offset[3];
	s->envelope_rate[4]   = 0; /* skip this stage */
	s->envelope_offset[5] = s->envelope_offset[4];
	s->envelope_rate[5]   = 0; /* skip this stage, then the voice is
				       disappeared */
#endif
	s->modes |= MODES_SUSTAIN;
	s->modes |= MODES_LOOPING;
#if BROKEN
/* Remove abnormal loops which cause pop noise
		 * in long sustain stage
		 */
		if (! (sp->modes & MODES_LOOPING)) {
			sp->loop_start = sp->data_length - 1;
			sp->loop_end = sp->data_length;
			sp->data[sp->data_length - 1] = 0;
		}

	

	int loop_start=(offset+consonant)*s->sample_rate/1000;
	int length_msec=length/s->sample_rate;
	int loop_end=loop_start+16;


	s->loop_end    =  loop_start << FRACTION_BITS;
	s->loop_start  =  loop_end << FRACTION_BITS;
#endif
	



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
}



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
	int size=header.data_chunk_size*2;
	//if(size % 0x1000 !=0) size += (0x1000 - (size % 0x1000));
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
	

	
	//pink part to be streched
	//blue part end and beginning
	//http://utau.wikia.com/wiki/File:How_To_Create_your_own_UTAU_voice_bank_rev0.40.pdf
	//offset (left blank)
	//consonant (fixed part) lengh
	//cutoff (right blank)

	//pre utterance,overlap TODO
	//red line -> previous option, sound start
	//green line -> overlap (vowals,n's,m's)
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
	int j;

	while(fgets(oto,sizeof(oto),f))
	{
	parse_oto(oto,&voicebank[i]);
	i++;
	}
	voicebank_count=i;

	printf("loading voicebank...");fflush(stdout);
	qsort(voicebank,voicebank_count,sizeof(Oto),oto_compare_name);
	for(i=0;i<voicebank_count;i++)
	{
	char wav_file[1024];
	sprintf(wav_file,"%s/%s",utau,voicebank[i].name);
	if(i>1 && strcmp(voicebank[i].name,voicebank[i-1].name)==0) voicebank[i].wavefile=voicebank[i-1].wavefile;
	else if((voicebank[i].wavefile=utau_read_riff(wav_file))==0) printf("failed to load %s\n",voicebank[i].name);
	if(voicebank[i].wavefile)
	{
		voicebank[i].sample=(Sample*)malloc(sizeof(Sample));
		memset(voicebank[i].sample,0,sizeof(Sample));
		utau_init_sample(i);	
		for(j=0;j<strlen(voicebank[i].name);j++)
			if(voicebank[i].name[j]=='.') voicebank[i].name[j]=0;
		
	}
	}
	qsort(voicebank,voicebank_count,sizeof(Oto),oto_compare_name);
	printf("done\n");
	
}

void utau_set_text(char* text)
{
	while(*text=='\\' || *text=='/' || *text==' ')
	text++;
	utau_text=text;
}

char* utau_get_text()
{
	return utau_text;
}

Sample* utau_get_sample(int* count)
{
	Oto* o=bsearch(utau_text,voicebank,voicebank_count,sizeof(Oto),oto_compare_name);
	if(o==0)
	{
	printf("[oto not found %s]\n",utau_text);
	*count=0;
	return 0;
	}
	else
	{
	*count=1;
	if(o->consonant||o->pre_utterance||o->overlap);
	printf("Warning: ingnoring %i %i %i for %s\n",o->consonant,o->pre_utterance,o->overlap,utau_text);
	return o->sample;
	}
}

int prescan_start;
void utau_prescan_on(int t)
{
	prescan_start=t;
}
void utau_prescan_off(int t)
{
	if(t==0) return;
	int length=t-prescan_start;
	if(length<22050) printf("UTAU prescan: %i %s is to shoort\n",length,utau_text);
	//1/4 ~ 22050
	//1/8 ~ 11025
}
void utau_prescan_lyr(char* t)
{
	utau_text=t;
}

void utau_mix(Sample* samp,int i)
{
#if 0
	int start=samp->loop_start >> FRACTION_BITS;
	int end=samp->loop_end >> FRACTION_BITS;
	//int length=samp->data_length >> FRACTION_BITS;
	int length=samp->data_length;
	if(samp->modes & MODES_LOOPING)
	{
	 printf("UTAU mix %i %i %i\n",start,end,i);
	 printf("modes looping\n");
	 utau_write_wave(samp->data+start,length-start-end,samp->sample_rate,"/tmp/test.wav");
	 exit(0);
        }
	//exit(0);
#endif
}

