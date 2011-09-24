//
//  OngakuUSTNote.m
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

#import "OngakuUSTNote.h"




@implementation OngakuUSTNote

@synthesize begin;
@synthesize end;
@synthesize pitch;

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
@end
