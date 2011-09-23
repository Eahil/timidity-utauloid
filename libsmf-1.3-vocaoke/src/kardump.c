#if 0
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

static gint
events_array_compare_function(gconstpointer aa, gconstpointer bb)
{
	smf_event_t *a, *b;
	
	/* "The comparison function for g_ptr_array_sort() doesn't take the pointers
	    from the array as arguments, it takes pointers to the pointers in the array." */
	a = (smf_event_t *)*(gpointer *)aa;
	b = (smf_event_t *)*(gpointer *)bb;

	if (a->time_pulses < b->time_pulses)
		return (-1);

	if (a->time_pulses > b->time_pulses)
		return (1);

	/*
	 * We need to preserve original order, otherwise things will break
	 * when there are several events with the same ->time_pulses.
	 * XXX: This is an ugly hack; we should remove sorting altogether.
	 */

	if (a->event_number < b->event_number)
		return (-1);

	if (a->event_number > b->event_number)
		return (1);

	return (0);
}

static char* get_lyric(smf_track_t* track)
{
	char* txt;
	smf_event_t* evt;
	do
	{
		evt=smf_track_get_next_event(track);
		if(evt==0) return 0;
		//if(((int)evt->midi_buffer[1])==3) evt=smf_track_get_next_event(track);//skip track name
		txt=smf_event_extract_text(evt);
		if(txt==0) return 0;
		//printf("txt %s %i\n",txt,(int)evt->midi_buffer[1]);
		
	} while(*txt=='@');
	if(*txt==' ' || *txt=='/' || *txt=='\\') return txt+1;
	return txt;
}

static int
get_tempo(smf_t* smf)
{
	int i;
	smf_tempo_t *tempo;
	double bpm=0;
	int count=0;

	for (i = 0;; i++) {
		tempo = smf_get_tempo_by_number(smf, i);
		if (tempo == NULL)
			break;

		g_message("Tempo #%d: Starts at %d pulses, %f seconds, setting %d microseconds per quarter note, %.2f BPM.",
		    i, tempo->time_pulses, tempo->time_seconds, tempo->microseconds_per_quarter_note,
		    60000000.0 / (double)tempo->microseconds_per_quarter_note);
		g_message("Time signature: %d/%d, %d clocks per click, %d 32nd notes per quarter note.",
		    tempo->numerator, tempo->denominator, tempo->clocks_per_click, tempo->notes_per_note);
		bpm=60000000.0 / (double)tempo->microseconds_per_quarter_note;
		count++;	
		
	}
	if(count!=1) return 0;
	else return bpm;
}


int main(int argc,char** argv)
{
	smf_t* smf=smf_load(argv[1]);
	FILE* xml=fopen("/tmp/lauloid.xml","w");
	
	float bpm=143;//hardcoded
	fprintf(xml,"<SINGING BPM=\"%f\">\n",bpm);
	float scale=1.0f/480.0f;
	
	
	
	smf_track_t* track1=smf_get_track_by_number(smf,1);//karaoke
	smf_track_t* track2=smf_get_track_by_number(smf,2);//karaoke2
	smf_track_t* track3=smf_get_track_by_number(smf,3);//lyrics
	smf_track_t* track4=smf_get_track_by_number(smf,4);//notes
	smf_event_t* event;

	
	GPtrArray* on=g_ptr_array_new();
	GPtrArray* off=g_ptr_array_new();
	GPtrArray* lyr=g_ptr_array_new();
	


	
		
	while ((event = smf_track_get_next_event(track4)) != NULL)
	{
		if(event->midi_buffer_length==3)
		{
			int cmd=(int)event->midi_buffer[0];	
			if(cmd==0x80) //note off
			{
				g_ptr_array_add(off,event);
			}
			else if(cmd==0x90) //note on
			{
				g_ptr_array_add(on,event);
			}
		}
	}
	g_ptr_array_sort(on, events_array_compare_function);
	g_ptr_array_sort(off, events_array_compare_function);
	while ((event = smf_track_get_next_event(track3)) != NULL)
	{
		if(smf_event_is_metadata(event))
		{
			char* txt=smf_event_extract_text(event);
			int type=(int)event->midi_buffer[1];
			if(txt && type==1 && *txt!='@')
			{
			 if(*txt==' '||*txt=='\\'||*txt=='/') txt++; 	
			 g_ptr_array_add(lyr,txt);
			}
		}
	}
	printf("%i %i %i\n",on->len,off->len,lyr->len);

	int timestamp=0;
	for(int i=0;i<on->len;i++)
	{
		smf_event_t* evton=g_ptr_array_index(on,i);
		smf_event_t* evtoff=g_ptr_array_index(off,i);
		char* lyric=g_ptr_array_index(lyr,i);
		if(lyric && evton->midi_buffer[1] == evtoff->midi_buffer[1])
		{
		printf("%i %i\n",evton->time_pulses,evtoff->time_pulses);
		if(evton->time_pulses > timestamp) fprintf(xml,"   <REST BEATS=\"%f\"></REST>\n",scale*(evton->time_pulses - timestamp));
		float nrest=0;
		if(evton->time_pulses < timestamp) {printf("negative rest %f",(evton->time_pulses - timestamp));nrest=(evton->time_pulses - timestamp);}
		int notenum=evton->midi_buffer[1];
		fprintf(xml,"   <PITCH NOTE=\"%s%i\"><DURATION BEATS=\"%f\">%s</DURATION></PITCH>\n",notenames[notenum%12],notenum/12,scale*(evtoff->time_pulses - evton->time_pulses+nrest),lyric);
		timestamp=evtoff->time_pulses;	
		
		}
	}	

	fprintf(xml,"</SINGING>");
	fclose(xml);
}
#endif
int main(){printf("broken tool");}
