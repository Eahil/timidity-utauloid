/*
    TiMidity++ (UTAU fork)

    Copyright (C) 2011 Tobias Platen	<tobias@platen-software.de>	
    Copyright (C) 1999-2004 Masanao Izumo <iz@onicos.co.jp>
    Copyright (C) 1995 Tuukka Toivonen <tt@cgs.fi>

    Special Thanks to Ameya/Ayame for creating UTAU.
    resampler.exe is similar to resample.c, it changes the pitch
    wavetool.exe is similar to mix.c, it concatenates and blends the wave files.
    UTAU.exe is the gui frontend, a GNU GPL'ed replacement has to be written.
    This patched version of TiMidiy++ can sing any midi file exported by UTAU.
    The documentation written by Kirk at
    http://utau.wikia.com/wiki/File:How_To_Create_your_own_UTAU_voice_bank_rev0.40.pdf
    was useful at writing this file.
    I wrote this program because UTAU does not work well on Unix Systems(broken wavetool) and European system locales.

    

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
#include <sndfile.h>

//TODO and note adjusting

static char* utau_text="default";

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

FILE* utau_xml=NULL;

#if 0
Note utau_notes[4000];
int utau_note_index=0;
int utau_note_count=0;
#endif

extern char* utau;//the utauloid voicebank path

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
    
    cutoff=0;
    s->data_length =  (length-offset-cutoff+2) << FRACTION_BITS;
    s->loop_end    =  (length-offset-cutoff+2) << FRACTION_BITS;
    s->loop_start  = s->loop_end - 0x1000;
    s->data=riff->samples+offset;
    s->panning= 63;
    s->volume= 10;
    s->modes= 3;//FIXME use envelope
    s->data_alloced= 1;
    s->scale_freq= 70;
    s->scale_factor= 1024;
    s->inst_type=INST_SF2;
    s->sample_type= 1;
    s->chord = -1;
    s->root_freq_detected = freq_fourier(s, &(s->chord));
    s->root_freq=s->root_freq_detected*1000;
    s->low_freq= s->root_freq/1.4;
    s->high_freq= s->root_freq*1.4;



//rate > OFFSET_MAX : instant
    /* envelope (0,1:attack, 2:sustain, 3,4,5:release) */
#if 1
#define OFFSET_MAX (0x3FFFFFFFL)
    s->modes |= MODES_ENVELOPE;
    int endoffs=env_offset(1);	
    /* attack */
    s->envelope_offset[0] = env_offset(255);
    s->envelope_rate[0]   = OFFSET_MAX+1;
    s->envelope_offset[1] = env_offset(255);
    s->envelope_rate[1]   = OFFSET_MAX+1; /* skip this stage */
    /* sustain */
    s->envelope_offset[2] = s->envelope_offset[1]*0;
    s->envelope_rate[2]   = 0;
    /* release */
    s->envelope_offset[3] = endoffs;
    s->envelope_rate[3] = OFFSET_MAX+1;	
    s->envelope_offset[4] = endoffs;
    s->envelope_rate[4] = OFFSET_MAX+1;
    s->envelope_offset[5] = endoffs;
    s->envelope_rate[5] = OFFSET_MAX+1;
#endif
    //envelope_velf == envelope velocity-follow	
    //s->modes |= MODES_SUSTAIN;
    //s->modes |= MODES_LOOPING;
    //vp->envelope_increment = (int32)rate;
    //vp->envelope_target = offset;
    //vp->envelope_keyf	//not for drums

//rate = tan(angle)
//offset = volume




}


Riff * utau_read_riff(char* filename)
{
	SF_INFO info={0};
	printf("will read file %s\n",filename);
	SNDFILE* sndfile = sf_open(filename,SFM_READ,&info);
	printf("%s has %i samples filename\n",filename,info.frames);
	Riff* r=(Riff*)malloc(sizeof(Riff));
	r->length=info.frames*2;//fixme dont hardcode
	r->samples=malloc(r->length);
	r->sample_rate=info.samplerate;
	sf_readf_short(sndfile,r->samples,info.frames);
	sf_close(sndfile);
	return r;   
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
    if(strcmp("xml",utau)==0)
    {	
	utau_xml=fopen("/tmp/utau.xml","w");
	fprintf(utau_xml,"<SINGING BPM=\"120\">\n");
	return ;
    }
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

Oto* utau_get_oto(char* name)
{
	Oto* o=bsearch(name,voicebank,voicebank_count,sizeof(Oto),oto_compare_name);
	return o;
}

Sample* utau_get_sample(int* count)
{
   if(utau_xml)
	{
		fprintf(utau_xml,"</SINGING>");
		exit(1);
	}

   // printf("UTAU: set text %s %s %i %i %i\n",utau_text,utau_notes[utau_note_index].text,utau_notes[utau_note_index].start,utau_notes[utau_note_index].end,utau_note_index);
   // utau_note_index++;
   // 
    Oto* o=utau_get_oto(utau_text);
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
MidiEvent* setup_start_event=0;
MidiEvent* setup_stop_event=0;
static int lyr_count=0;
static int note_count=0;
static char* utau_prescan_text=0;
static int is_utau_midifile=0;
//MidiEvent* setup_event=0;
void utau_prescan_on(MidiEvent* e)
{
    if(e->time==0) setup_start_event=e;	
    start_event=e;	
}

char* notenames[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

int utau_lastend=0;

void utau_prescan_off(MidiEvent* e)
{
    	
    if(utau_xml)
    {
		float scale = 1.0/22050;
		float beats = scale*(e->time - start_event->time);
		int note = MIDI_EVENT_NOTE(e);
		int rest = start_event->time - utau_lastend;float restf=1.0*rest/22050;
		utau_lastend=e->time;
		if(rest) fprintf(utau_xml,"<REST BEATS=\"%f\"></REST>\n",restf);
		fprintf(utau_xml,"<PITCH NOTE=\"%s%i\"><DURATION BEATS=\"%f\">%s</DURATION></PITCH>\n",notenames[note%12],note/12,beats,utau_prescan_text);
		return ; 
    }
    		
    note_count++;
    if(note_count>lyr_count) 
    {
	printf("missing lyrics for note %i\n",note_count);
	exit(0);
    }
    if(e->time==0) setup_stop_event=e;
    else
    {
	if(is_utau_midifile==1)
	{
 	setup_start_event->type=ME_PROGRAM;
	setup_stop_event->type=ME_PROGRAM;
	is_utau_midifile=2;
	}
	//utau produces a setup event
    }	
    	
    int p=2000;
    //if(e->time>p) e->time-=p;
	
}
//fixme alias in oto.ini is broken
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
    else is_utau_midifile=1;	
    lyr_count++;	

}

/////
//#0  0x0805b267 in recompute_envelope ()
//#1  0x08071b31 in start_note ()
//#2  0x08077acc in play_event ()
//#3  0x0807948a in play_midi_file ()


