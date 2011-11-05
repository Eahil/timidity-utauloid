//
//  OngakuScoreView.m
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

#import "OngakuScoreView.h"
#import "OngakuUSTNote.h"
#import "OngakuScore.h"
#include "smf.h"
#include "dictionary.h"
#include "iniparser.h"

#define DRAG_CREATE 0
#define DRAG_MOVE 1
#define DRAG_MOVE_LEFT 2
#define DRAG_MOVE_RIGHT 3

//Import/Export
//OngakuScore(UTAU,VOICALOID,XML,MIDI,NSKeyedArchiver)

//OngakuUSTPlayer
//OngakuScore

//fluidsynth backend 

//native format ? 

//midi output devices
//	OngakuMIDIOutput
//  OngakuMIDIInput

//TODO implement snap to grid for notes

//USE cases from UTAU docs
//2-1. Setting the tempo
//2-6. Changing the length of notes and rests
//2-2. Entering notes and lyrics
//
//STATUS bar: show number of selected notes

//SELECT tracks with 1 .. 4



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

@synthesize score;

- (id)initWithFrame:(NSRect)frame {
	frame.size.height=18*127;
	frame.size.width=8000;//FIXME do not hardcode
    self = [super initWithFrame:frame];
    if (self) {
        keyHeight=18;
		quarterLength=50;
		//notes=[[NSMutableArray alloc]init];
		score=[[OngakuScore alloc]initWithMidiFile:@"/home/kakashi/Desktop/finlandia1.mid"];
		notes=[score notes];
		NSLog(@"notes count = %i",[notes count]);
		editor=nil;
		
    }
	[NSTimer scheduledTimerWithTimeInterval:1.0/50 target:self selector:@selector(timerEvent:) userInfo:nil repeats:YES];
    return self;
	
}

-(void)timerEvent:(id)anObject
{
	if(editor) [[self window]makeFirstResponder:editor];
	else
	{
	[self setNeedsDisplay:YES];
	[[self window]makeFirstResponder:self];
	}
	
}


- (void)drawRect:(NSRect)dirtyRect {
	
	static int t=0;
	t++;
	int length=8*16+1;
	
	
	NSRect bound = [self bounds];
	[[NSColor grayColor]set];
	[NSBezierPath fillRect:bound];
	
	
	for(int i=0;i<127;i++)
	{
		
		[pianoRollColor(i) set];
		[NSBezierPath fillRect:NSMakeRect(0,keyHeight*i, bound.size.width, keyHeight-2)];
		
	}
	for(int i=0;i<length;i++)
	{
		
		if(i%4==0)//FIXME
			[[NSColor redColor]set];	
		else
			[[NSColor darkGrayColor]set];
		
			[NSBezierPath fillRect:NSMakeRect(quarterLength*i,0, 1, bound.size.height)];
		
	}
	
	
	for(OngakuUSTNote* note in notes)		
	{
		[[note color]set];
		[note drawRect:NSMakeRect(quarterLength*note.begin,note.pitch*keyHeight, quarterLength*(note.end-note.begin), keyHeight)];
	}
	
	
	//draw now playing
	[[NSColor greenColor]set];
	
    
}

#pragma mark editing

- (void)mouseDown:(NSEvent *)theEvent
{
	drag=NO;
	NSPoint event_location = [theEvent locationInWindow];
	start_point = [self convertPoint:event_location fromView:nil];
	
	int pitch=start_point.y/keyHeight;
	
	
	
	
	
	if(editor)
	{
		NSString* s=[editor string];
		if([s rangeOfString:@" "].location==NSNotFound)
			[(OngakuUSTNote*)[editor delegate]setLyric:s];
		else
		{
			smf_t* smftmp=smf_new();
			smf_track_t* tracktmp=smf_track_new();
			smf_add_track(smftmp,tracktmp);
			
			NSMutableArray* tmp=[NSMutableArray new];
			for(NSString* s2 in [s componentsSeparatedByCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"- \n"]])
			{
				NSLog(@"lyr=%@",s2);
				[tmp addObject:s2];
				
			}
			float begin=((OngakuUSTNote*)[editor delegate]).begin;
			int track=((OngakuUSTNote*)[editor delegate]).track;

			NSLog(@"aaa");
			
			int i=0;
			for(OngakuUSTNote* note in notes)
			{
				if(note.begin>=begin && note.track==track)
				{
					if(i<[tmp count])
					{
						NSString* s3=[tmp objectAtIndex:i];
						i++;
						[note setLyric:s3];
						NSLog(@"s3 %@",s3);
						smf_event_t* evtlyr=smf_event_new_textual(SMF_TEXT_TYPE_LYRIC,[s3 cString]);
		 				char notenum=note.pitch;
		  				smf_event_t* evton=smf_event_new_from_bytes(0x90,notenum,127);
		  				smf_event_t* evtoff=smf_event_new_from_bytes(0x80,notenum,0);
		  				
		  				double ond=(note.begin);
		  				double offd=(note.end);
						smf_track_add_event_seconds(tracktmp,evtlyr,ond);
		  				smf_track_add_event_seconds(tracktmp,evton,ond);			  				
		  				smf_track_add_event_seconds(tracktmp,evtoff,offd);	
					}
				}
			}

			
			smf_save(smftmp,"/tmp/lauloid.mid");
			//(voice_ogi_abc_diphone)(Flinger.sing "z:/tmp/lauloid.mid" nil)(quit)
			
			
			
		}
		[editor removeFromSuperview];
		[editor release];
		editor=nil;
	}
	
	if([theEvent clickCount]==2) NSLog(@"doubleclick");
	
	
	for(OngakuUSTNote* note in notes)	
	{
		NSLog(@"note:");	
		if(fabs(note.begin*quarterLength - start_point.x)<2 && note.pitch==pitch)
		{
			[[NSCursor resizeLeftCursor]set];
			dragAction=DRAG_MOVE_LEFT;
			noteUnderCursor=note;
			return;
		}
		if(fabs(note.end*quarterLength - start_point.x)<2 && note.pitch==pitch)
		{
			[[NSCursor resizeRightCursor]set];
			dragAction=DRAG_MOVE_RIGHT;
			noteUnderCursor=note;
			return;
		}
		else
		if(note.pitch==pitch && note.begin < start_point.x/quarterLength && note.end > start_point.x/quarterLength) 
		{
			if([theEvent clickCount]==2)
			{
			NSLog(@"%@",editor);
			editor=[[NSTextView alloc]initWithFrame:NSMakeRect(quarterLength*note.begin,note.pitch*keyHeight, quarterLength*(note.end-note.begin), keyHeight)];
			#ifndef __APPLE__
			[editor retain];
			#endif
			NSLog(@"%@",editor);
			[editor setDelegate:note];
			[editor setBackgroundColor:[NSColor colorWithDeviceRed:0.7 green:0.3 blue:1.0 alpha:1.0]]; 
			[editor setString:[note.lyric string]];
			[self addSubview:editor];
			}
			else {
				noteUnderCursor=note;
				dragAction=DRAG_MOVE;				
			}

			return;
		}
	}
	
	//TODO allow dragging existing notes
	dragAction = DRAG_CREATE;
	noteUnderCursor=[OngakuUSTNote new];
	[notes addObject:noteUnderCursor];
	
}
- (void) mouseUp:(NSEvent *)theEvent
{
	//NSPoint event_location = [theEvent locationInWindow];
	//NSPoint local_point = [self convertPoint:event_location fromView:nil];
	//TODO remove invalid notes
	
	
	if(dragAction==DRAG_CREATE && drag) [noteUnderCursor setLyric:@"A"];
	if(drag==0)
	{
		[noteUnderCursor toggleSelection];
		if(!([theEvent modifierFlags] & NSShiftKeyMask))
			for(OngakuUSTNote* note in notes)
				if(note!=noteUnderCursor)
					[note deselect];
		
	}
	if(noteUnderCursor.end<noteUnderCursor.begin) [notes removeObject:noteUnderCursor];
	noteUnderCursor=nil;
	[[NSCursor arrowCursor]set];
	
	
	//TODO sort notes
}

-(void) mouseDragged:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	NSPoint local_point = [self convertPoint:event_location fromView:nil];
	
	drag=YES;
	
	if(dragAction==DRAG_CREATE)
	{
		noteUnderCursor.begin=start_point.x/quarterLength;
		noteUnderCursor.end=local_point.x/quarterLength;
		noteUnderCursor.lyric=[NSString stringWithFormat:@"%f",noteUnderCursor.end-noteUnderCursor.begin];
		noteUnderCursor.pitch=start_point.y/keyHeight;
	}
	if(dragAction==DRAG_MOVE)
	{
		noteUnderCursor.pitch=local_point.y/keyHeight;
	}
	if(dragAction==DRAG_MOVE_LEFT)
	{
		noteUnderCursor.begin=local_point.x/quarterLength;
	}
	if(dragAction==DRAG_MOVE_RIGHT)
	{
		noteUnderCursor.end=local_point.x/quarterLength;
	}
	
	
	
}

-(void) rightMouseDown:(NSEvent *)theEvent
{
	//TODO showContextMenu
}

-(void)keyUp:(NSEvent *)theEvent
{
	int keycode=[theEvent keyCode];
	NSLog(@"k=%i",keycode);
	if(keycode==117)//delete
	{
		NSMutableIndexSet* idx=[NSMutableIndexSet indexSet];
		for(int i=0;i<[notes count];i++)
		{
			if([[notes objectAtIndex:i]isSelected]) [idx addIndex:i];
		}
		[notes removeObjectsAtIndexes:idx];
	}
	if(keycode==125)
	{
		for(OngakuUSTNote* note in notes)
		{
			if([note isSelected]) note.pitch-=1;
		}
	}
	if(keycode==126)
	{
		for(OngakuUSTNote* note in notes)
		{
			if([note isSelected]) note.pitch+=1;
		}
	}
	if(keycode==123)
	{
		for(OngakuUSTNote* note in notes)
		{
			if([note isSelected]) 
			{
				note.begin-=1;
				note.end-=1;
			}
		}
	}
	if(keycode==124)
	{
		for(OngakuUSTNote* note in notes)
		{
			if([note isSelected]) 
			{
				note.begin+=1;
				note.end+=1;
			}
		}
	}
			
	//TODO move shift notes
}
-(void) keyDown:(NSEvent *)theEvent
{
	
}
#if 0
- (BOOL)canBecomeKeyView
{
	return YES;
}
#endif





//TODO allow editing lyrics
//TODO 
//TODO more utau view functions

@end
