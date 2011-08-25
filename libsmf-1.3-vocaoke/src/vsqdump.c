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
#include "iniparser.h"


//convert vsq track to dictionary
dictionary* smf_decode_vsq(smf_t *smf,int track)
{
	smf_track_t *selected_track = smf_get_track_by_number(smf, track);

	if (selected_track->number_of_events == 0) {
		g_message("Selected track is empty.");
		return NULL;
	}
	
	smf_event_t *event;	
	smf_rewind(smf);
	char **buffer=malloc(sizeof(char*)*100000);
	buffer[0]=malloc(256+1);
	memset(buffer[0],0,256);	
	int l=0;
	int j=0;		
		
	while ((event = smf_track_get_next_event(selected_track)) != NULL)
	{
		if(smf_event_is_metadata(event) && event->midi_buffer[1]==1)
		{
			for(int i=3+8;i<event->midi_buffer_length;i++)
			{
				char c=event->midi_buffer[i];
				
				switch(c)
				{
				case '\r':
				case '\n':
					printf("%s\n",buffer[l]);
					l++;
					j=0;
					buffer[l]=malloc(256+1);
					memset(buffer[l],0,256);
				break;
				case '#':
					buffer[l][j]='@';
					if(j>254) goto error;
					j++;
				break;
				default:
					buffer[l][j]=c;
					if(j>254) goto error;
					j++;				 			
				}
				
			}
		}
		
	}
	return iniparser_load_lines(buffer,l-1);
	//FIXME: free buffer
	error:
		printf("BUFFER overflow\n");
		exit(0);
		
	
}


	

int main(int argc,char** argv)
{
	if(argc!=2) return fprintf(stderr,"usage: vsqdump file.vsq\n");
	smf_t *smf = smf_load(argv[1]);
	if(smf==NULL) return fprintf(stderr,"error: could not read %s\n",argv[1]);

	smf_t* smfnew=smf_new();
	smf_track_t *newtrack = smf_track_new();
	smf_add_track(smfnew,newtrack);	

	
	
	
	dictionary* root=smf_decode_vsq(smf,2);

	smf_rewind(smf);
	smf_rewind(smfnew);
	/*void write_time_tempo */{
		char data[7];
		//FF 58 04 nn dd cc bb
	        data[0]=0xFF;
		data[1]=0x58;
		data[2]=0x04;
		data[3]=4;//nominator
		data[4]=2;//nominator
		data[5]=24;
		data[6]=8;
		int musec=1000*500;
		smf_event_t *timesig=smf_event_new_from_pointer(&data,7);
		assert(smf_event_is_valid(timesig));
		
		//FF 51 03 tttttt
		data[1]=0x51;
		data[2]=0x03;
		data[3]=(musec>>16 & 0xFF);
		data[4]=(musec>>8 & 0xFF);
		data[5]=(musec & 0xFF);	
		smf_event_t *tempo=smf_event_new_from_pointer(&data,6);
		assert(smf_event_is_valid(tempo));
		
		printf("%s\n",smf_event_decode(timesig));
		printf("%s\n",smf_event_decode(tempo));
		smf_track_add_event_seconds(newtrack,timesig,0);
		smf_track_add_event_seconds(newtrack,tempo,0);
		//exit(0);
		
	}

	int secs=0;
        for(int i=0;i<root->n;i++)
	{
		//3: Metadata: Time Signature: 4/4, 24 clocks per click, 8 notated 32nd notes per quarter note, 0.000000 seconds, 0 pulses, 0 delta pulses
		//4: Metadata: Tempo: 555555 microseconds per quarter note, 108.00 BPM, 0.000000 seconds, 0 pulses, 0 delta pulses
		char* key=root->key[i];
		char* value=root->val[i];
		char* keyref="eventlist:";
		char* k2=strstr(key,keyref);
		
		if(k2)
		{ 
		  k2+=strlen(keyref);
		  char* lh=iniparser_getstring2(root,value,"lyrichandle","");
		  char* note=iniparser_getstring2(root,value,"note@","");
		  char* len=iniparser_getstring2(root,value,"length","");
		  char* lyr=iniparser_getstring2(root,lh,"L0","");
		  if(strlen(len))
		  {

		  
		  smf_event_t* evtlyr=smf_event_new_textual(SMF_TEXT_TYPE_LYRIC,lyr);
		  char notenum=(char)atoi(note);
		  smf_event_t* evton=smf_event_new_from_bytes(0x90,notenum,127);
		  smf_event_t* evtoff=smf_event_new_from_bytes(0x80,notenum,0);
		  int on=atoi(k2);
		  int off=on+atoi(len);
		  double ond=((double) on)/960;
		  double offd=((double) off)/960;
		  smf_track_add_event_seconds(newtrack,evton,ond);	
		  smf_track_add_event_seconds(newtrack,evtlyr,ond);
		  smf_track_add_event_seconds(newtrack,evtoff,offd);
		  //printf("%s %s %s at %i .. %i : %f .. %f \n",lyr,note,len,on,off,ond,offd);

		  }	
		  

		   	
		   	
 		  	
		  //480 ~ 1/4
		  //1920 ~  1	
		  
		  
		}
		
		
	} 
	printf("save midi\n");	
	smf_save(smfnew,"/tmp/autosave.mid");
	system("timidity /tmp/autosave.mid");	

	return (0);
		
}
