//
//  OngakuUSTNote.m
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
