//
//  OngakuAppDelegate.h
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

@class OngakuScoreView;

@interface OngakuAppDelegate : NSObject /*<NSApplicationDelegate>*/ {
    NSWindow *window;
	OngakuScoreView* *view;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet OngakuScoreView *scoreView;
-(IBAction) openDocument:(id)sender;
@end
