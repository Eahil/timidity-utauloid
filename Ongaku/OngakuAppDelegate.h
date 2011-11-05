//
//  OngakuAppDelegate.h
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class OngakuScoreView;

@interface OngakuAppDelegate : NSObject /*<NSApplicationDelegate>*/ {
    NSWindow *window;
	OngakuScoreView* *view;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet OngakuScoreView *scoreView;

@end
