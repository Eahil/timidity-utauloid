//
//  OngakuUSTNote.m
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

#import "OngakuUSTNote.h"



//COLOR cyan - miku 1
//COLOR yellow - rin/len 2
//COLOR magenta - luka 3
//COLOR blue - kaito
//COLOR red - akaito
//COLOR brown - meiko


@implementation OngakuUSTNote

@synthesize begin;
@synthesize end;
@synthesize pitch;
@synthesize track;

-(void) drawRect:(NSRect)rect
{
	[NSBezierPath fillRect:rect];
	[lyric drawInRect:rect];
}
+(id) noteWithPitch:(int)pitch begin:(float)begin end:(float)end
{
	OngakuUSTNote* n=[[OngakuUSTNote new]autorelease];
	n.begin=begin;
	n.end=end;
	n.pitch=pitch;
	return n;
	
}
-(void)setLyric:(id)newLyric
{
	[lyric release];
	if(newLyric)
	lyric = [[NSAttributedString alloc]initWithString:newLyric];
	else
	lyric = nil;

}
-(id)lyric
{
	return lyric;
}
-(void) toggleSelection
{
	selected=~selected;
}
-(NSColor*) color
{
	if(selected)
		return [NSColor blueColor];
	else
	{
		//if([lyric length]==0)
		//	return [NSColor darkGrayColor];
		switch (track) {
			case 0:
				return [NSColor redColor];
			case 1:
				return [NSColor cyanColor];
			case 2:
				return [NSColor yellowColor];
			case 3:
				return [NSColor magentaColor];
			case 4:
				return [NSColor brownColor];
			default:
				return [NSColor darkGrayColor];
		}
	}
}
-(BOOL) isSelected
{
	return selected;
}
-(void) deselect
{
	selected=NO;
}

@end
