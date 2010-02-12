
/*
 * This is simply because OpenGL includes are different on mac and other platforms. This is just to make sure Aria is as portable as possible.
 */

#ifdef __WXMAC__

#include "OpenGL/glu.h"
#include "OpenGL/gl.h"

#else

#include <GL/glu.h>
#include <GL/gl.h>

#endif
