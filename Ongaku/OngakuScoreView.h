//
//  OngakuScoreView.h
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface OngakuScoreView : NSView {
	
@private
	int keyHeight;
	int quarterLength;
	NSMutableArray* notes;

}

@end
