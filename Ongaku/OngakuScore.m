//
//  OngakuScore.m
//  Ongaku
//
//  Created by Tobias Platen on 02.11.11.
//
/*
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "OngakuScore.h"
#import "OngakuUSTNote.h"

#include "smf.h"
#include "dictionary.h"
#include "iniparser.h"

static int ld2(int i)
{
	int ret=-1;
	while(i>0)
	{
		i/=2;
		ret+=1;
	}
	return ret;
}

void write_tempo(smf_track_t* newtrack,smf_tempo_t* tempo) {
	char data[7];
	//FF 58 04 nn dd cc bb
	data[0]=0xFF;
	data[1]=0x58;
	data[2]=0x04;
	data[3]=tempo->numerator;
	data[4]=ld2(tempo->denominator);
	data[5]=tempo->clocks_per_click;
	data[6]=tempo->notes_per_note;
	int musec=tempo->microseconds_per_quarter_note;
	smf_event_t *timesig=smf_event_new_from_pointer(&data,7);
	assert(smf_event_is_valid(timesig));
	
	//FF 51 03 tttttt
	data[1]=0x51;
	data[2]=0x03;
	data[3]=(musec>>16 & 0xFF);
	data[4]=(musec>>8 & 0xFF);
	data[5]=(musec & 0xFF);
	smf_event_t *evttempo=smf_event_new_from_pointer(&data,6);
	assert(smf_event_is_valid(evttempo));
	
	printf("%s\n",smf_event_decode(timesig));
	printf("%s\n",smf_event_decode(evttempo));
	smf_track_add_event_seconds(newtrack,timesig,0);
	smf_track_add_event_seconds(newtrack,evttempo,0);
	//exit(0);
	
}




@implementation OngakuScore
@synthesize notes;
-(id) init
{
	notes=[NSMutableArray new];
	return self;	
}

-(id) initWithMidiFile:(NSString*)fileName
{
	[self init];
	
	OngakuUSTNote* tmp[128];
	memset(tmp,0,sizeof(tmp));
	smf_t* smf=smf_load([fileName UTF8String]);
	smf_event_t *event;
	
	tempo=smf_get_last_tempo(smf);
	
	for(int i=0;i<smf->number_of_tracks;i++)
	{
		smf_track_t* track=smf_get_track_by_number(smf,1+i);
		while ((event = smf_track_get_next_event(track)) != NULL)
		{
			if(event->midi_buffer_length==3)
			{
				int cmd=(int)event->midi_buffer[0];
				int arg1=(int)event->midi_buffer[1];
				int arg2=(int)event->midi_buffer[2];
				int type;
				cmd = cmd & 0xF0;
				if(cmd==0x80) //note off
				{
					type=0;
				}
				else if(cmd==0x90) //note on
				{
					type=1;
					if(arg2==0) type=0;	
				}
				else printf("cmd = %x",cmd);
				if(type==1) {
					
					tmp[arg1]=[OngakuUSTNote new];
					tmp[arg1].pitch=arg1;
					tmp[arg1].begin=event->time_seconds;
					tmp[arg1].track=i;
				}
				else {
					
					tmp[arg1].end=event->time_seconds;
					//NSLog(@"add note %@",tmp[arg1]);
					if(tmp[arg1]) [notes addObject:tmp[arg1]];
					tmp[arg1]=nil;
					
				}
				//ignore B0
				
			}
		}
	}
	return self;
	
}

-(id) initWithUSTFile:(NSString*)fileName
{
	[self init];
	dictionary* root=iniparser_load((char*)[fileName UTF8String]);
	int i=0;
	int cur=0;
	while(1)
	{
		char sect[6];
		sprintf(sect,"#%04i",i);
		char* lyric=iniparser_getstring2(root,sect,"Lyric","");
		char* note=iniparser_getstring2(root,sect,"NoteNum","");
		char* length=iniparser_getstring2(root,sect,"Length","");
		i++;
		if(strlen(lyric)==0) break;
		int start=cur;
		cur+=atoi(length);
		int end=cur;
		
		if(strcmp(lyric,"R"))
		{
			NSLog(@"got lyric %s",lyric);
			float scale=1.0/480.0;	
			OngakuUSTNote* n=[OngakuUSTNote noteWithPitch:atoi(note) begin:start*scale end:end*scale];
			n.lyric=[NSString stringWithUTF8String:lyric];
			[notes addObject:n];
			
		}
	}
	return self;
}
-(void) exportSelection
{
	smf_t* smf=smf_new();
	smf_track_t* track=smf_track_new();
	smf_add_track(smf, track);
	
	write_tempo(track,tempo);
	
	char inst[2];
	inst[0]=0xC0;
	inst[1]=0x20;
	smf_event_t* changeinst=smf_event_new_from_pointer(&inst,2);
	smf_track_add_event_seconds(track,changeinst,0);
	
	
	for(OngakuUSTNote* note in notes)
	{
		//NSLog(@"%@",note);
		char pitch = note.pitch;
		smf_event_t* evtbegin=smf_event_new_from_bytes(0x90, pitch, 127);
		smf_event_t* evtend=smf_event_new_from_bytes(0x80, pitch, 0);
		smf_track_add_event_seconds(track,evtbegin,note.begin);
		smf_track_add_event_seconds(track,evtend,note.end);
	}
	
	smf_save(smf,"/tmp/lauloid.mid");
}

@end
