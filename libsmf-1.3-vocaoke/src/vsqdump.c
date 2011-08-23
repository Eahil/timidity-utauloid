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
#include <stdio.h>
#include "smf.h"
#include "config.h"

char* smf_decode_vsq(smf_t *smf,int track)
{
	smf_track_t *selected_track = smf_get_track_by_number(smf, track);

	if (selected_track->number_of_events == 0) {
		g_message("Selected track is empty.");
		return (0);
	}
	
	smf_event_t *event;	

	smf_rewind(smf);
	int it=0;
	while ((event = smf_track_get_next_event(selected_track)) != NULL)
	{
		if(smf_event_is_metadata(event) && event->midi_buffer[1]==1)
		{
			for(int i=3+8;i<event->midi_buffer_length;i++)
			{
				//printf("%c %i\n",event->midi_buffer[i],i);
				//printf("%c",event->midi_buffer[i],i);
				it++;				
			}
		}
		
	}
	smf_rewind(smf);
	if(it==0) return 0;
	char* ret=malloc(it+1);
	if (ret==0) {
		g_message("malloc failed\n");
		return (0);
	}
        it=0;
	while ((event = smf_track_get_next_event(selected_track)) != NULL)
	{
		if(smf_event_is_metadata(event) && event->midi_buffer[1]==1)
		{
			for(int i=3+8;i<event->midi_buffer_length;i++)
			{
				//printf("%c %i\n",event->midi_buffer[i],i);
				//printf("%c",,i);
				ret[it]=event->midi_buffer[i];
				it++;				
			}
		}
		
	}
	ret[it]=0;
	smf_rewind(smf);
	return ret;
}


int main(int argc,char** argv)
{
	if(argc!=2) return fprintf(stderr,"usage: vsqdump file.vsq\n");
	smf_t *smf = smf_load(argv[1]);
	if(smf==NULL) return fprintf(stderr,"error: could not read %s\n",argv[1]);
	//char* decoded=smf_decode(argv[1]);
	printf("%s",smf_decode_vsq(smf,2));
	

	return (0);
		
}
