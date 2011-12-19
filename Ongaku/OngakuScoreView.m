//
//  OngakuScoreView.m
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
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

#import "OngakuScoreView.h"
#import "OngakuUSTNote.h"
#import "OngakuScore.h"
#include "smf.h"
#include "dictionary.h"
#include "iniparser.h"
#include "OngakuKeyboardView.h"

#define DRAG_CREATE 0
#define DRAG_MOVE 1
#define DRAG_MOVE_LEFT 2
#define DRAG_MOVE_RIGHT 3

enum {
	kVK_ANSI_A                    = 0x00,
	kVK_ANSI_S                    = 0x01,
	kVK_ANSI_D                    = 0x02,
	kVK_ANSI_F                    = 0x03,
	kVK_ANSI_H                    = 0x04,
	kVK_ANSI_G                    = 0x05,
	kVK_ANSI_Z                    = 0x06,
	kVK_ANSI_X                    = 0x07,
	kVK_ANSI_C                    = 0x08,
	kVK_ANSI_V                    = 0x09,
	kVK_ANSI_B                    = 0x0B,
	kVK_ANSI_Q                    = 0x0C,
	kVK_ANSI_W                    = 0x0D,
	kVK_ANSI_E                    = 0x0E,
	kVK_ANSI_R                    = 0x0F,
	kVK_ANSI_Y                    = 0x10,
	kVK_ANSI_T                    = 0x11,
	kVK_ANSI_1                    = 0x12,
	kVK_ANSI_2                    = 0x13,
	kVK_ANSI_3                    = 0x14,
	kVK_ANSI_4                    = 0x15,
	kVK_ANSI_6                    = 0x16,
	kVK_ANSI_5                    = 0x17,
	kVK_ANSI_Equal                = 0x18,
	kVK_ANSI_9                    = 0x19,
	kVK_ANSI_7                    = 0x1A,
	kVK_ANSI_Minus                = 0x1B,
	kVK_ANSI_8                    = 0x1C,
	kVK_ANSI_0                    = 0x1D,
	kVK_ANSI_RightBracket         = 0x1E,
	kVK_ANSI_O                    = 0x1F,
	kVK_ANSI_U                    = 0x20,
	kVK_ANSI_LeftBracket          = 0x21,
	kVK_ANSI_I                    = 0x22,
	kVK_ANSI_P                    = 0x23,
	kVK_ANSI_L                    = 0x25,
	kVK_ANSI_J                    = 0x26,
	kVK_ANSI_Quote                = 0x27,
	kVK_ANSI_K                    = 0x28,
	kVK_ANSI_Semicolon            = 0x29,
	kVK_ANSI_Backslash            = 0x2A,
	kVK_ANSI_Comma                = 0x2B,
	kVK_ANSI_Slash                = 0x2C,
	kVK_ANSI_N                    = 0x2D,
	kVK_ANSI_M                    = 0x2E,
	kVK_ANSI_Period               = 0x2F,
	kVK_ANSI_Grave                = 0x32,
	kVK_ANSI_KeypadDecimal        = 0x41,
	kVK_ANSI_KeypadMultiply       = 0x43,
	kVK_ANSI_KeypadPlus           = 0x45,
	kVK_ANSI_KeypadClear          = 0x47,
	kVK_ANSI_KeypadDivide         = 0x4B,
	kVK_ANSI_KeypadEnter          = 0x4C,
	kVK_ANSI_KeypadMinus          = 0x4E,
	kVK_ANSI_KeypadEquals         = 0x51,
	kVK_ANSI_Keypad0              = 0x52,
	kVK_ANSI_Keypad1              = 0x53,
	kVK_ANSI_Keypad2              = 0x54,
	kVK_ANSI_Keypad3              = 0x55,
	kVK_ANSI_Keypad4              = 0x56,
	kVK_ANSI_Keypad5              = 0x57,
	kVK_ANSI_Keypad6              = 0x58,
	kVK_ANSI_Keypad7              = 0x59,
	kVK_ANSI_Keypad8              = 0x5B,
	kVK_ANSI_Keypad9              = 0x5C
};

//Import/Export
//OngakuScore(UTAU,VOICALOID,XML,MIDI,NSKeyedArchiver)

//OngakuUSTPlayer


//Output
//fluidsynth backend 
//cspeak
//flite


//midi output devices
//	OngakuMIDIOutput
//  OngakuMIDIInput

 



//USE cases from UTAU docs
//2-1. Setting the tempo
//2-6. Changing the length of notes and rests
//2-2. Entering notes and lyrics

//Tempo Display Area
//Editing Quantization Unit
//Length of inserted notes
//Lyrics Textbox->create Menu Entry
//Lyrics Replace Button

//Note Insert Button
//Rest Insert Button

//Play / Pause / Stop buttons


//STATUS bar: show number of selected notes
//TODO implement snap to grid for notes
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

static int pianoRollLength(int i)
{
	switch (i%12) {
		case 0://C
		case 2://D
		case 4://E
		case 5://F
		case 7://G
		case 9://A
		case 11://B
			[[NSColor whiteColor]set];
			return 100;		
		default:
			[[NSColor blackColor]set];
			return 50;
	}
	
}


@implementation OngakuScoreView

@synthesize score;
@synthesize keyboardView;

- (id)initWithFrame:(NSRect)frame {
	frame.size.height=18*127;//FIXME
	frame.size.width=8000;//FIXME do not hardcode
    self = [super initWithFrame:frame];
    if (self) {
        keyHeight=18;
		quarterLength=50;
		//notes=[[NSMutableArray alloc]init];
		//score=[[OngakuScore alloc]initWithMidiFile:@"/home/kakashi/Desktop/finlandia1.mid"];
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

#if 0
- (void)awakeFromNib
{
    NSScrollView *scrollView = [self enclosingScrollView];
	
	//[self addSubview:[[NSScroller alloc]initWithFrame:NSMakeRect(0, 0, 1000, 20)]];
	//return 0;
	if (!scrollView) return;
	[NSScrollView setRulerViewClass:[OngakuKeyboardView class]];
    //[scrollView setHasHorizontalRuler:YES];
    [scrollView setHasVerticalRuler:YES];
	[scrollView setRulersVisible:YES];
	
	
	
    return;
}
#endif


- (void)drawRect:(NSRect)dirtyRect {
	
	
	float abs=fabs(lastDirtyRect.origin.y-dirtyRect.origin.y);
	if(abs>0)
	{
		NSLog(@"scroll: %f %f",abs,dirtyRect.origin.y);
		[keyboardView shift:dirtyRect.origin.y keyHeight:keyHeight];
	}
	
	lastDirtyRect=dirtyRect;
	
	
	static int t=0;
	t++;
	
	//FIXME
	
	
	
	
	
	NSRect bound = [self bounds];
	[[NSColor grayColor]set];
	[NSBezierPath fillRect:bound];
	int length=((int)(bound.size.width/quarterLength))+1;
	
	
	for(int i=0;i<127;i++)
	{
		//[[NSColor greenColor]set];

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
	
	return;
	
	int keyHeight=18;
	[[NSColor grayColor]set];
	[NSBezierPath fillRect:dirtyRect];
	
	
	//	[[NSColor redColor]set];
	//	[NSBezierPath fillRect:NSMakeRect(0,1000-y, 200, 1)];
	
	
	
    
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
	if(keycode==kVK_ANSI_S)
	{
		[score exportSelection];
	}
			
	//TODO move shift notes
}
-(void) keyDown:(NSEvent *)theEvent
{
	
}
-(void) loadScore:(NSString*)fileName
{
	NSLog(@"%@",fileName);
	score=[[OngakuScore alloc]initWithMidiFile:fileName];
	notes=[score notes];
	NSRect rect=[self frame];
	rect.size.width=16000;
	[self setFrame:rect];
	[self setNeedsDisplay:YES];
	
	
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
