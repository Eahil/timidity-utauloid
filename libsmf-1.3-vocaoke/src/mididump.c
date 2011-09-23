#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef __MINGW32__
#define EX_OK 0
#define EX_USAGE 64
#else /* ! __MINGW32__ */
#include <sysexits.h>
#endif /* ! __MINGW32__ */
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "smf.h"

const char* notenames[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

int main()
{
	smf_t* smf=smf_load("/tmp/autosave.mid");
	FILE* xml=fopen("/tmp/lauloid.xml","w");
	float bpm=180;
	fprintf(xml,"<SINGING BPM=\"%f\">\n",bpm);
	
	smf_track_t* track=smf_get_track_by_number(smf,1);
	smf_event_t* event;

	

	int notenum=-1;
	int dynamics=-1;
	char* lyric=NULL;
	float start;	
	float end=-1;

		

	while ((event = smf_track_get_next_event(track)) != NULL)
	{
		if(smf_event_is_metadata(event))
		{
			//handle lyric events
			lyric = smf_event_extract_text(event);
			//handle other meta events
		}
		else
		{
			//handle note on/off
			if(event->midi_buffer_length==3)
			{
				int cmd=(int)event->midi_buffer[0];
				int arg1=(int)event->midi_buffer[1];
				int arg2=(int)event->midi_buffer[2];
				float seconds=event->time_seconds;
				
				if(cmd==0x80) //note off
				{
					//use notenames here
					//printf("%4i %3i %5s {}\n",pulses-start,notenum,lyric);
					float beats=(seconds-start)*bpm/60;
					fprintf(xml,"   <PITCH NOTE=\"%s%i\"><DURATION BEATS=\"%f\">%s</DURATION></PITCH>\n",notenames[notenum%12],notenum/12,beats,lyric);					
					notenum=-1;
					dynamics=-1;
					end=seconds;	
					lyric=0;
				}
				else if(cmd==0x90) //note on
				{
					if(notenum!=-1) printf("error\n");
					if(end!=seconds && end!=-1)
					{
                     			 float restf=(seconds-end)*bpm/60;
					 fprintf(xml,"   <REST BEATS=\"%f\"></REST>\n",restf);
					 //printf("%4i R\n",pulses-end);
					}
					notenum=arg1;
					dynamics=arg2;
					start=seconds;
					
				}
				//0xAn relative to start time,Polyphonic Aftertouch
				//Control Change,Program Change
				//0xEn Pitch Bending
				//not allowed Monophonic / Channel Aftertouch
				//TODO encode other events		
			}
		}
	}
	fprintf(xml,"</SINGING>");
	fclose(xml);
}
