//
//  OngakuUSTNote.h
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

#import <Cocoa/Cocoa.h>




@interface OngakuUSTNote : NSObject /*<NSTextViewDelegate>*/{
	float begin;
	float end;
	int pitch;//0 to 127
	NSAttributedString* lyric;
	BOOL selected;
	int track;//1 to 4
	//NSColor color;
	//int voicebank
}

-(void) drawRect:(NSRect)rect;
+(id) noteWithPitch:(int)pitch begin:(float)begin end:(float)end;
-(void) toggleSelection;
-(NSColor*) color;
-(BOOL) isSelected;
@property float begin;
@property float end;
@property int pitch;
@property int track;
@property(assign) id lyric;



@end
