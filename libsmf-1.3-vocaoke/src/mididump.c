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

int main()
{
	smf_t* smf=smf_load("/tmp/autosave.mid");
	
	smf_track_t* track=smf_get_track_by_number(smf,1);

	smf_event_t* event;

	

	int notenum=-1;
	int dynamics=-1;
	char* lyric=NULL;
	int start;	
	int end=-1;

	while ((event = smf_track_get_next_event(track)) != NULL)
	{
		if(smf_event_is_metadata(event))
		{
			//handle lyric events
			lyric = smf_event_extract_text(event);
		}
		else
		{
			//handle note on/off
			if(event->midi_buffer_length==3)
			{
				int cmd=(int)event->midi_buffer[0];
				int arg1=(int)event->midi_buffer[1];
				int arg2=(int)event->midi_buffer[2];
				int pulses=event->time_pulses;
				
				if(cmd==0x80) //note off
				{
					
					int l=printf("%4i %3i %5s {}\n",pulses-start,notenum,lyric);					
					notenum=-1;
					dynamics=-1;
					end=pulses;	
					
				}
				else if(cmd==0x90) //note on
				{
					if(notenum!=-1) printf("error\n");
					if(end!=pulses && end!=-1) printf("%4i R\n",pulses-end);
					notenum=arg1;
					dynamics=arg2;
					start=pulses;
				}	
			}
		}
	}
}
