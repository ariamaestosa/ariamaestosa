"""SCons.Tool.gcc

Tool-specific initialization for gcc.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

"""

#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 The SCons Foundation
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

__revision__ = "src/engine/SCons/Tool/gcc.py 3266 2008/08/12 07:31:01 knight"

import SCons.Util

import cc
import os
import re

compilers = ['gcc', 'cc']

def generate(env):
    """Add Builders and construction variables for gcc to an Environment."""
    cc.generate(env)

    env['CC'] = env.Detect(compilers) or 'gcc'
    if env['PLATFORM'] in ['cygwin', 'win32']:
        env['SHCCFLAGS'] = SCons.Util.CLVar('$CCFLAGS')
    else:
        env['SHCCFLAGS'] = SCons.Util.CLVar('$CCFLAGS -fPIC')
    # determine compiler version
    if env['CC']:
        line = os.popen(env['CC'] + ' --version').readline()
        match = re.search(r'[0-9]+(\.[0-9]+)+', line)
        if match:
            env['CCVERSION'] = match.group(0)

def exists(env):
    return env.Detect(compilers)
