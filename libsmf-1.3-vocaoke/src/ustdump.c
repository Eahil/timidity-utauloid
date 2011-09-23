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


void write_tempo(smf_track_t* newtrack,float bpm) {
   char data[7];
   //FF 58 04 nn dd cc bb
   data[0]=0xFF;
	data[1]=0x58;
	data[2]=0x04;
	data[3]=4;//nominator
	data[4]=2;//nominator
	data[5]=24;
	data[6]=8;
	int musec=1000*1000*60/bpm;
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

int main(int argc,char** argv)
{
	if(argc!=2) return fprintf(stderr,"usage: ustdump file.ust\n");
	dictionary* root=iniparser_load(argv[1]);

	
	#if 0
	for(int i=0;i<root->n;i++)
	{
		//3: Metadata: Time Signature: 4/4, 24 clocks per click, 8 notated 32nd notes per quarter note, 0.000000 seconds, 0 pulses, 0 delta pulses
		//4: Metadata: Tempo: 555555 microseconds per quarter note, 108.00 BPM, 0.000000 seconds, 0 pulses, 0 delta pulses
		char* key=root->key[i];
		char* value=root->val[i];
		printf("%s = %s\n",key,value);
	}
	#endif

	smf_track_t* track=smf_track_new();
	smf_t* smf=smf_new();
	smf_add_track(smf,track);	
	float tempo=atof(iniparser_getstring(root,"#SETTING:Tempo",""));
	printf("%f tempo\n",tempo);
	write_tempo(track,tempo);

	if(0){
	        smf_event_t* evtlyr=smf_event_new_textual(SMF_TEXT_TYPE_LYRIC,"(Setup)");
		char notenum=60;
		smf_event_t* evton=smf_event_new_from_bytes(0x90,notenum,127);
		smf_event_t* evtoff=smf_event_new_from_bytes(0x80,notenum,0);
		smf_track_add_event_seconds(track,evton,0);	
		smf_track_add_event_seconds(track,evtlyr,0);
		smf_track_add_event_seconds(track,evtoff,0);
	}
	
	
	int i=0;
	int cur=0;
	while(1)
	{
		char sect[6];
		sprintf(sect,"#%04i",i);
		char* lyric=iniparser_getstring2(root,sect,"Lyric","");
		char* note=iniparser_getstring2(root,sect,"NoteNum","");
		char* length=iniparser_getstring2(root,sect,"Length","");
		if(strlen(lyric)==0) break;
		int start=cur;
  		cur+=atoi(length);
      		int end=cur;
		
		if(strcmp(lyric,"R"))
		{
		   smf_event_t* evtlyr=smf_event_new_textual(SMF_TEXT_TYPE_LYRIC,lyric);
		   char notenum=(char)atoi(note);
		   smf_event_t* evton=smf_event_new_from_bytes(0x90,notenum,127);
		   smf_event_t* evtoff=smf_event_new_from_bytes(0x80,notenum,0);

		  double ond=((double) start)/(8*tempo);
		  double offd=((double) end)/(8*tempo);
		  smf_track_add_event_seconds(track,evtlyr,ond);
		  smf_track_add_event_seconds(track,evton,ond);	
		  smf_track_add_event_seconds(track,evtoff,offd);	

		
		   printf("sect: %s %s %s %s %i .. %i\n",sect,lyric,note,length,start,end);
		}
		else
		{
				
			printf("sect: %s [REST] %s %s %i .. %i\n",sect,note,length,start,end);
			
		}
		i++;
	}
	smf_save(smf,"/tmp/autosave.mid");
	system("timidity /tmp/autosave.mid");		
	
		
}
