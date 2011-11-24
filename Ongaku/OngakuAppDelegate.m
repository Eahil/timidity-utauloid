//
//  OngakuAppDelegate.m
//  Ongaku
//
//  Created by Tobias Platen on 22.09.11.
//  Copyright 2011 THM Giessen. All rights reserved.
//

#import "OngakuAppDelegate.h"



@implementation OngakuAppDelegate

@synthesize window;
@synthesize scoreView;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 
}
- (void)windowWillClose:(NSNotification *)notification
{
	NSLog(@"close %@",scoreView);
	[[scoreView score]writeToFile:@"/tmp/score.dat"];//TODO write to score
	[NSApp terminate:nil];
}

-(IBAction) openDocument:(id)sender
{
	NSOpenPanel* p=[NSOpenPanel openPanel];
	[p runModal];
	[scoreView loadScore:[[p filenames]objectAtIndex:0]];
}

@end
