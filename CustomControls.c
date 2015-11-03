/*
        IMPORTANT: This Apple software is supplied to you by Apple Computer,
        Inc. ("Apple") in consideration of your agreement to the following terms,
        and your use, installation, modification or redistribution of this Apple
        software constitutes acceptance of these terms.  If you do not agree with
        these terms, please do not use, install, modify or redistribute this Apple
        software.
        
        In consideration of your agreement to abide by the following terms, and
        subject to these terms, Apple grants you a personal, non-exclusive
        license, under Apple’s copyrights in this original Apple software (the
        "Apple Software"), to use, reproduce, modify and redistribute the Apple
        Software, with or without modifications, in source and/or binary forms;
        provided that if you redistribute the Apple Software in its entirety and
        without modifications, you must retain this notice and the following text
        and disclaimers in all such redistributions of the Apple Software.
        Neither the name, trademarks, service marks or logos of Apple Computer,
        Inc. may be used to endorse or promote products derived from the Apple
        Software without specific prior written permission from Apple. Except as
        expressly stated in this notice, no other rights or licenses, express or
        implied, are granted by Apple herein, including but not limited to any
        patent rights that may be infringed by your derivative works or by other
        works in which the Apple Software may be incorporated.
        
        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
        NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
        IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
        PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
        ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
        
        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
        SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
        MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
        WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
        LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
        OF SUCH DAMAGE.  
*/

// ----------------------------------------------------------------------
// Includes

#include <Carbon/Carbon.h>


pascal SInt32 MyControlDefProc(	SInt16 varCode, ControlHandle theControl,
	ControlDefProcMessage message, SInt32 param);

pascal OSStatus MyControlCNTLToCollectionProc( const Rect * bounds, SInt16 value,
	Boolean visible, SInt16 max, SInt16 min, SInt16 procID, SInt32 refCon,
	ConstStr255Param title, Collection collection);

pascal OSStatus MyEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

static const EventTypeSpec kControlEventList[] =
{
	{ kEventClassControl, kEventControlDraw }
};

//————————————————————————————————————————————————————————————————————————————
//	• RegisterCustomControls
//————————————————————————————————————————————————————————————————————————————

void RegisterCustomControls()
{
    ControlDefSpec		defSpec;
    ToolboxObjectClassRef	myControlClassRef;

    // Register my Standard Defproc
    defSpec.defType = kControlDefProcPtr;
    defSpec.u.defProc = NewControlDefUPP( MyControlDefProc );
    RegisterControlDefinition( 500, &defSpec, NewControlCNTLToCollectionUPP( MyControlCNTLToCollectionProc ) );

    // Register my event based custom control
    RegisterToolboxObjectClass(CFSTR("MyControl"), 
                                NULL, 1, kControlEventList, 
                                NewEventHandlerUPP( MyEventHandler ), 
                                NULL, &myControlClassRef);

}

//————————————————————————————————————————————————————————————————————————————
//	• MyControlDefProc
//————————————————————————————————————————————————————————————————————————————
//	The entrypoint for our custom Control Definition.
//	We can and must do all of the stuff that we would have done within a
//	'CDEF' resource.
//
//	This particular Control Definition simply draws its title within the
//	control's bounds.
//
pascal SInt32 MyControlDefProc(	SInt16 varCode, ControlHandle theControl,
	ControlDefProcMessage message, SInt32 param)
{
#pragma unused( varCode )

	SInt32 				result = 0;
	Rect				bounds;
	Str255				title;
	GrafPtr				currentPort;
	ThemeDrawingState               state;
	SInt16				savedFont;
	Style				savedFace;
	SInt16				savedMode;
	SInt16				savedSize;
	RgnHandle			region;
        HIRect                          frame;
        
        HIViewGetBounds(theControl, &frame);
        bounds.top = frame.origin.y;
        bounds.left = frame.origin.x;
        bounds.bottom = frame.origin.y + frame.size.height;
        bounds.right = frame.origin.x + frame.size.width;
        
//	GetControlBounds( theControl, &bounds );
	GetControlTitle( theControl, title );

	switch ( message ) {
		case drawCntl:
			// prepare to draw in the current port
			GetPort( &currentPort );
			savedFont = GetPortTextFont( currentPort );
			savedFace = GetPortTextFace( currentPort );
			savedMode = GetPortTextMode( currentPort );
			savedSize = GetPortTextSize( currentPort );
			GetThemeDrawingState( &state );
			NormalizeThemeDrawingState();
			UseThemeFont( kThemeSystemFont, smSystemScript );

			// just draw our title within our bounding box
			TETextBox( &title[1], title[0], &bounds, teFlushDefault );
                        FrameRect(&bounds);

			// restore what we did to the port
			SetThemeDrawingState( state, true );
			TextFont( savedFont );
			TextFace( savedFace );
			TextMode( savedMode );
			TextSize( savedSize );
			break;

		case testCntl:
			// we are display only, so we don't track
			result = kControlNoPart;
			break;

		case initCntl:
			// we don't do any special work during initialization
			// and we return noErr as an indication that initialization
			// was successful.
			result = noErr;
			break;

		case dispCntl:
			// we don't have to do any work
			break;

		case calcCntlRgn:
			// this Control Definition is as big as its bounds
			region = (RgnHandle)param;
			RectRgn( region, &bounds );
			break;

		case kControlMsgGetFeatures:
			// we have no features
			result = 0;
			break;

		case kControlMsgTestNewMsgSupport:
			// we support the new messages, so return
			// the appropriate value
			result = kControlSupportsNewMessages;
			break;

		default:
			break;
	}

	return result;
}

//————————————————————————————————————————————————————————————————————————————
//	• MyControlCNTLToCollectionProc
//————————————————————————————————————————————————————————————————————————————
//	All controls are now created through a new API (CreateNewCustomControl)
//	which does not take explicit value, min, max, refCon, or other parameters.
//	Instead, it takes a Collection which can have all of that information
//	along with any special info which is unique to each Control Definition.
//	Unfortunately, calls to NewControl and GetNewControl only have access to
//	the basic value, min, max information. Because those pieces of information
//	might be overloaded to have a special meaning for our Control Definition,
//	the Control Manager needs to know how to translate that data into the right
//	tagged Collection data. We have registered this routine to do the
//	translation for our custom Control Definition.
//
pascal OSStatus MyControlCNTLToCollectionProc( const Rect * bounds, SInt16 value,
	Boolean visible, SInt16 max, SInt16 min, SInt16 procID, SInt32 refCon,
	ConstStr255Param title, Collection collection)
{
#pragma unused( procID )

	OSStatus		err = noErr;
	SInt32		value32 = value;
	SInt32		max32 = max;
	SInt32		min32 = min;

	// The value, min, etc. do not get overloaded into special meanings for us,
	// so we can simply add each one to the collection with the standard Control
	// Collection Tags. The Control Manager will recognize these standard tags
	// and will give their values to the control instance.

	err = AddCollectionItem( collection, kControlCollectionTagBounds, 0,
		sizeof( Rect ), (void*)bounds );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagValue, 0,
		sizeof( SInt32 ), &value32 );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagVisibility, 0,
		sizeof( Boolean ), &visible );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagMaximum, 0,
		sizeof( SInt32 ), &max32 );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagMinimum, 0,
		sizeof( SInt32 ), &min32 );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagRefCon, 0,
		sizeof( SInt32 ), &refCon );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagTitle, 0,
		title[0], (void*)&title[1] );
	if ( err != noErr ) goto CantAddCollectionItem;

CantAddCollectionItem:
	
	return err;
}

//————————————————————————————————————————————————————————————————————————————
//	• MyEventHandler
//————————————————————————————————————————————————————————————————————————————

pascal OSStatus MyEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
    OSStatus result = noErr;
    ControlRef		theControl; 
    HIRect              bounds;
    CGContextRef context = NULL;
    
    switch ( GetEventKind( inEvent ) )
    {
        case kEventControlDraw:            
            GetEventParameter( inEvent, kEventParamDirectObject, typeControlRef, NULL,
		sizeof( ControlRef ), NULL, &theControl );
            GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof( CGContextRef ), NULL, &context);
                        
            HIViewGetBounds( theControl, &bounds );
            CGContextSetRGBFillColor( context, 1, 0, 0, 0.25 );
            CGContextSetRGBStrokeColor( context, 1, 0, 0, 1 );
            CGContextFillRect( context, bounds );
            CGContextStrokeRect( context, bounds );
            break;
    }
    
    return result;
}
