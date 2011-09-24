//
//  OngakuUSTNote.h
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

#import <Cocoa/Cocoa.h>




@interface OngakuUSTNote : NSObject {
	float begin;
	float end;
	int pitch;//0 to 127
	NSAttributedString* lyric;
	//NSColor color;
	//int voicebank
}

-(void) drawRect:(NSRect)rect;
+(id) noteWithPitch:(int)pitch begin:(float)begin end:(float)end;
@property float begin;
@property float end;
@property int pitch;
@property id lyric;


@end
