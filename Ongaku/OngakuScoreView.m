//
//  OngakuScoreView.m
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

#import "OngakuScoreView.h"
#import "OngakuUSTNote.h"
#include "smf.h"
#include "dictionary.h"
#include "iniparser.h"



static NSColor* pianoRollColor(int i)
{
	switch (i%12) {
		case 0://C
		case 2://D
		case 4://E
		case 5://F
		case 7://G
		case 9://A
		case 11://B
			return [NSColor whiteColor];		
		default:
		return [NSColor lightGrayColor];	
	}
	
}


@implementation OngakuScoreView

- (id)initWithFrame:(NSRect)frame {
	frame.size.height=18*127;
    self = [super initWithFrame:frame];
    if (self) {
        keyHeight=18;
		quarterLength=50;
		notes=[[NSMutableArray alloc]init];
		
		dictionary* root=iniparser_load("/tmp/test.ust");
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
    }
	[NSTimer scheduledTimerWithTimeInterval:1.0/50 target:self selector:@selector(timerEvent:) userInfo:nil repeats:YES];
    return self;
	
}

-(void)timerEvent:(id)anObject
{
	[self setNeedsDisplay:YES];
}


- (void)drawRect:(NSRect)dirtyRect {
	
	static int t=0;
	t++;
	
	
	NSRect bound = [self bounds];
	[[NSColor grayColor]set];
	[NSBezierPath fillRect:bound];
	
	
	for(int i=0;i<127;i++)
	{
		
		[pianoRollColor(i) set];
		[NSBezierPath fillRect:NSMakeRect(0,keyHeight*i, bound.size.width, keyHeight-2)];
		
	}
	for(int i=0;i<18;i++)
	{
		
		if(i%4==0)
			[[NSColor redColor]set];	
		else
			[[NSColor darkGrayColor]set];
		
			[NSBezierPath fillRect:NSMakeRect(quarterLength*i,0, 1, bound.size.height)];
		
	}
	
	
	
	
	
	for(OngakuUSTNote* note in notes)		
	{
		[[NSColor colorWithDeviceRed:0.0 green:0.5 blue:1.0 alpha:1.0]set];
		[note drawRect:NSMakeRect(quarterLength*note.begin,note.pitch*keyHeight, quarterLength*(note.end-note.begin), keyHeight)];
	}
	
	
	//draw now playing
	[[NSColor greenColor]set];
	
	[NSBezierPath fillRect:NSMakeRect(t,0, 1, bound.size.height)];
	
	
	 
	
	
	
    
}

@end
