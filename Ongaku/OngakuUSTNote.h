//
//  OngakuUSTNote.h
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

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
