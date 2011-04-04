

#include "GUI/Machelper.h"

#ifdef __WXOSX_COCOA__

#import <AppKit/NSButtonCell.h>
#import <AppKit/NSToolbar.h>

/*
 enum {
 NSRoundedBezelStyle           = 1,
 NSRegularSquareBezelStyle     = 2,
 NSThickSquareBezelStyle       = 3,
 NSThickerSquareBezelStyle     = 4,
 NSDisclosureBezelStyle        = 5,
 NSShadowlessSquareBezelStyle  = 6,
 NSCircularBezelStyle          = 7,
 NSTexturedSquareBezelStyle    = 8,
 NSHelpButtonBezelStyle        = 9,
 NSSmallSquareBezelStyle       = 10,
 NSTexturedRoundedBezelStyle   = 11,
 NSRoundRectBezelStyle         = 12,
 NSRecessedBezelStyle          = 13,
 NSRoundedDisclosureBezelStyle = 14,
 }
 */
void skinButton(WX_NSView view_p)
{
    NSView* view = view_p;
    NSButton* btn = (NSButton*)view;
    [btn setBezelStyle:NSRoundRectBezelStyle];//NSTexturedRoundedBezelStyle];
}

void skinToolbar(WX_NSView view_p)
{
    NSToolbar* tb = (NSToolbar*)view_p;
    [tb setAllowsUserCustomization:YES];
}

#endif