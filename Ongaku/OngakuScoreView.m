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

#define DRAG_CREATE 0
#define DRAG_MOVE 1
#define DRAG_MOVE_LEFT 2
#define DRAG_MOVE_RIGHT 3


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
	int length=8+1;
	
	
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
	
	if([theEvent clickCount]==2) NSLog(@"doubleclick");
	
	
	
	if(editor)
	{
		[(OngakuUSTNote*)[editor delegate]setLyric:[editor string]];
		[editor removeFromSuperview];
		[editor release];
		editor=nil;
	}
	
	
	
	
	for(OngakuUSTNote* note in notes)	
	{
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
			editor=[[NSTextView alloc]initWithFrame:NSMakeRect(quarterLength*note.begin,note.pitch*keyHeight, quarterLength*(note.end-note.begin), keyHeight)];
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
	if(keycode==117)
	{
		NSMutableIndexSet* idx=[NSMutableIndexSet indexSet];
		for(int i=0;i<[notes count];i++)
		{
			if([[notes objectAtIndex:i]isSelected]) [idx addIndex:i];
		}
		[notes removeObjectsAtIndexes:idx];
	}
	//TODO move shift notes
}
-(void) keyDown:(NSEvent *)theEvent
{
	
}

- (BOOL)canBecomeKeyView
{
	return YES;
}






//TODO allow editing lyrics
//TODO 
//TODO more utau view functions

@end