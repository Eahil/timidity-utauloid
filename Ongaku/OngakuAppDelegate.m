//
//  OngakuAppDelegate.m
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
