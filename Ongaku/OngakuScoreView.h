//
//  OngakuScoreView.h
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
@class OngakuUSTNote;
@class OngakuScore;
@class OngakuKeyboardView;
//TODO add shiftable 


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
	NSRect lastDirtyRect;
	OngakuKeyboardView* keyboardView;
}
@property(readonly) OngakuScore* score;
@property IBOutlet OngakuKeyboardView* keyboardView;
@end
