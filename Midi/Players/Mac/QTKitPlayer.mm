/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef _MAC_QUICKTIME_COREAUDIO

#include <iostream>

#import <Foundation/Foundation.h>
#import <Foundation/NSError.h>
#import <QTKit/QTMovie.h>
#import <QTKit/QTDataReference.h>

#import "QTKitPlayer.h"


QTMovie* movie = nil;
bool playing = false;

void qtkit_init()
{
}
void qtkit_free()
{
}

void qtkit_setData(char* data_bytes, int bytes_length)
{
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    NSData* data = [NSData dataWithBytes:data_bytes length:bytes_length];
    [data retain];
	
    QTDataReference* qtDataRef = [QTDataReference dataReferenceWithReferenceToData:data name:@".mid" MIMEType:@"MIDI"];
    [qtDataRef retain];
    
    
    NSError* err = nil;
    movie = [QTMovie movieWithDataReference:qtDataRef error: &err];
    [movie retain];
    
    
        
    if(err != nil)
	{
        printf("error:\n-->%s\n-->%s\n",
               [[err localizedDescription] UTF8String],
               [[err localizedRecoverySuggestion] UTF8String]);   
    }
    

    [data release];
    [qtDataRef release];
    
    [pool release];
}

void qtkit_play()
{
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
/*
    if(movie == nil)
	{
		printf("Movie is nil!\n");
		return;
	}
*/
	
    [movie play];
    
    playing = true;
    
    [pool release];
}

bool qtkit_exportToAiff(char* filename)
{
    printf("QTKit will save to %s\n", filename);
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithBool:YES], QTMovieExport, 
        [NSNumber numberWithLong:kQTFileTypeAIFF], 
        QTMovieExportType, nil];
    [dictionary retain];
    
	//NSString* nsstring_filepath = [NSString stringWithCString:"/Users/mathieu/Desktop/caut.aiff"];
    NSString* nsstring_filepath = [NSString stringWithCString:filename encoding:NSUTF8StringEncoding ];
    [nsstring_filepath retain];
	if(nsstring_filepath == nil)
	{
		printf("path could not be converted to NSString, aborting\n");
		return false;
	}
    
    bool success = [movie writeToFile:nsstring_filepath withAttributes:dictionary];
    
    [nsstring_filepath release];
    [dictionary release];
    
    [pool release];
	return success;
}

void qtkit_stop()
{
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    [movie stop];
    [movie release];
    movie = nil;
    playing = false;
    
    [pool release];
}

float qtkit_getCurrentTime()
{
    
    if(!playing)
        return -1;
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    QTTime time = [movie currentTime];

    TimeRecord timeRec;
    QTGetTimeRecord( time, &timeRec );
    
    long hi = timeRec.value.hi;
    long lo = timeRec.value.lo;
    
    if(hi!=0){
        std::cout << "ERROR: too long" << std::endl;   
    }
    
    
    [pool release];
    
    return (float)lo / (float)timeRec.scale;

}

#endif