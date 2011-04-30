/*
    TiMidity++ (UTAU fork)

    Copyright (C) 2011 Tobias Platen	<tobias@platen-software.de>	
    Copyright (C) 1999-2004 Masanao Izumo <iz@onicos.co.jp>
    Copyright (C) 1995 Tuukka Toivonen <tt@cgs.fi>

    Special Thanks to Ameya/Ayame for creating UTAU.
	UTAU was not disassembled by me as the usage policy and copyright laws forbid this.

	see http://utau.wikia.com/wiki/UTAU_wiki:UTAU_Usage_Policy for more infomation.

	Instead the documentation written by Kirk at
	http://utau.wikia.com/wiki/File:How_To_Create_your_own_UTAU_voice_bank_rev0.40.pdf
	was useful at writing this file.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

    utau.c -- the singing synthesizer	(UTAU is japanese and means to sing)
*/
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
#include "utau.h"

//TODO and note adjusting

static char* utau_text="default";


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

typedef struct _Note
{
    int start;
    int end;
    char* text;
} Note;


Oto voicebank[4000];
int voicebank_count;

#if 0
Note utau_notes[4000];
int utau_note_index=0;
int utau_note_count=0;
#endif

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
#if 1
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


static int32 calc_rate(int diff, double msec)
{
    double rate;

    if(msec < 6)
	msec = 6;
    if(diff == 0)
	diff = 255;
    diff <<= (7+15);
    rate = ((double)diff / play_mode->rate) * control_ratio * 1000.0 / msec;
    return (int32)rate;
}




void utau_init_sample(Oto* o)
{
    Sample* s=o->sample;
    if(s->root_freq>0) return ;
    Riff* riff=o->wavefile;
    int offset=o->offset;
    int consonant=o->offset;
    int cutoff=o->cutoff;
    int pre_utterance=o->pre_utterance;
    int overlap=o->overlap;
    int length = (riff->length/2);
    s->sample_rate=riff->sample_rate;

    if(pre_utterance > 0 || overlap >0 )
    printf("WARNING OTO IGNORED: %s: pre utterance: %i  overlap: %i offset: %i consonant:%i \n",o->name,pre_utterance,overlap,offset,consonant);

    //offset+=consonant;
    offset*=riff->sample_rate;
    offset/=1000;
    cutoff*=riff->sample_rate;
    cutoff/=1000;

    //offset=0;//ignore offset
    cutoff=0;
    //offset/=2;



    s->data_length =  (length-offset-cutoff+2) << FRACTION_BITS;
    s->loop_end    =  (length-offset-cutoff+2) << FRACTION_BITS;
    s->loop_start  = s->loop_end - 0x1000;
    s->data=riff->samples+offset;

    s->panning= 63;
    s->volume= 20;
    s->modes= 3;//FIXME use envelope
    s->data_alloced= 1;

    s->scale_freq= 70;
    s->scale_factor= 1024;
    s->inst_type= INST_PCM;//not a soundfont
    s->sample_type= 1;
    s->chord = -1;
    s->root_freq_detected = freq_fourier(s, &(s->chord));
    s->root_freq=s->root_freq_detected*1000;
    s->low_freq= s->root_freq/1.4;
    s->high_freq= s->root_freq*1.4;

    //->inst_type == INST_SF2

    //printf("root frq in hz: %f\n",((float)s->root_freq)/1000);



    /* envelope (0,1:attack, 2:sustain, 3,4,5:release) */
#if 1
    s->modes |= MODES_ENVELOPE;
    int rate1=env_offset(255);
    int rate2=env_offset(255);
    int rate3=env_offset(0);
    int endoffs=env_offset(255);	
    /* attack */
    s->envelope_offset[0] = env_offset(255);
    s->envelope_rate[0]   = rate1;
    s->envelope_offset[1] = env_offset(255);
    s->envelope_rate[1]   = 0; /* skip this stage */
    /* sustain */
    s->envelope_offset[2] = s->envelope_offset[1];
    s->envelope_rate[2]   = 0;
    /* release */
    s->envelope_offset[3] = endoffs;
    s->envelope_rate[3] = rate3;
    s->envelope_offset[4] = endoffs;
    s->envelope_rate[4] = 0;
    s->envelope_offset[5] = endoffs;
    s->envelope_rate[5] = 0;
#endif
    //s->modes |= MODES_SUSTAIN;
    //s->modes |= MODES_LOOPING;




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
    sscanf(oto,"%s %s %i %i %i %i %i",&o->name,&o->alias,&o->offset,&o->consonant,&o->cutoff,&o->pre_utterance,&o->overlap);



    //pink part to be streched
    //blue part end and beginning
    //
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

    qsort(voicebank,voicebank_count,sizeof(Oto),oto_compare_name);
    for(i=0;i<voicebank_count;i++)
    {
	char wav_file[1024];
	sprintf(wav_file,"%s/%s",utau,voicebank[i].name);
	voicebank[i].wavefile=0;
	if(i>1 && strcmp(voicebank[i].name,voicebank[i-1].name)==0) voicebank[i].wavefile=voicebank[i-1].wavefile;
	else if((voicebank[i].wavefile=utau_read_riff(wav_file))==0) printf("failed to load %s\n",voicebank[i].name);
	if(voicebank[i].wavefile)
	{
            voicebank[i].sample=(Sample*)malloc(sizeof(Sample));
            memset(voicebank[i].sample,0,sizeof(Sample));
            //utau_init_sample(i);
            for(j=0;j<strlen(voicebank[i].name);j++)
                if(voicebank[i].name[j]=='.') voicebank[i].name[j]=0;

	}
    }
    qsort(voicebank,voicebank_count,sizeof(Oto),oto_compare_name);

}

void utau_set_text(char* text)
{
    if(strcmp(text,"(Setup)")==0) return;	
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
   
   // printf("UTAU: set text %s %s %i %i %i\n",utau_text,utau_notes[utau_note_index].text,utau_notes[utau_note_index].start,utau_notes[utau_note_index].end,utau_note_index);
   // utau_note_index++;
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
	//if(o->consonant||o->pre_utterance||o->overlap);
	//printf("Warning: ingnoring %i %i %i for %s\n",o->consonant,o->pre_utterance,o->overlap,utau_text);
	return o->sample;
    }
}

MidiEvent* start_event=0;
static int lyr_count=0;
static int note_count=0;
static char* utau_prescan_text;
//MidiEvent* setup_event=0;
void utau_prescan_on(MidiEvent* e)
{
    start_event=e;
    printf("%i note on %i note_count \n",MIDI_EVENT_NOTE(e),note_count);	
}
void utau_prescan_off(MidiEvent* e)
{
    note_count++;
    if(note_count>lyr_count) 
    {
	printf("missing lyrics for note %i\n",note_count);
	exit(0);
    }
    printf("%i note off\n",MIDI_EVENT_NOTE(e));		
    //int length=t-prescan_start;
    //if(length<22050) printf("UTAU prescan: %i %s is to shoort\n",length,utau_text);
	
}
void utau_prescan_lyr(char* t)
{
    utau_prescan_text=t;
    if(strcmp(t,"(Setup)"))
    {	
    Oto* o=bsearch(utau_prescan_text,voicebank,voicebank_count,sizeof(Oto),oto_compare_name);
    if(o)
        utau_init_sample(o);
    else
	printf("bad sample\n");
    }
    lyr_count++;	

}
static int utau_last_voice=1;
void utau_finnish_note(int i)
{
	if(utau_last_voice==i)
	{
	//voice[i].cache=NULL;//leak ?
	printf("BAD OVERLAP %i\n",i);
	}
	utau_last_voice=i;
}


