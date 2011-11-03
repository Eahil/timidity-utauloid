//
//  OngakuScoreView.h
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class OngakuUSTNote;
@class OngakuScore;


@interface OngakuScoreView : NSView {
	
@private
	int keyHeight;
	int quarterLength;
	NSMutableArray* notes;
	NSPoint start_point;
	OngakuUSTNote* noteUnderCursor;
	NSTextView* editor;
	int dragAction;
	BOOL drag;
	OngakuScore* score;

}
@property(readonly) OngakuScore* score;
@end
