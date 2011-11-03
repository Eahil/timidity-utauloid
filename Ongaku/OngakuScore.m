//
//  OngakuScore.m
//  Ongaku
//
//  Created by Tobias Platen on 02.11.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

#import "OngakuScore.h"
#import "OngakuUSTNote.h"

#include "smf.h"
#include "dictionary.h"
#include "iniparser.h"


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
	OngakuUSTNote* tmp=NULL;
	smf_t* smf=smf_load([fileName UTF8String]);
	smf_event_t *event;
	for(int i=0;i<4;i++)
	{
		smf_track_t* track=smf_get_track_by_number(smf,2+i);
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
					if(tmp) NSLog(@"overlapping note");
					tmp=[OngakuUSTNote new];
					tmp.pitch=arg1;
					tmp.begin=event->time_seconds;
					tmp.track=i;
				}
				else {
					if(tmp)
					{
						tmp.end=event->time_seconds;
						[notes addObject:tmp];
						tmp=nil;
					}
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

@end
