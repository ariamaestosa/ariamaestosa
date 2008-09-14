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

"""SCons.Errors

This file contains the exception classes used to handle internal
and user errors in SCons.

"""

__revision__ = "src/engine/SCons/Errors.py 3266 2008/08/12 07:31:01 knight"



class BuildError(Exception):
    def __init__(self, node=None, errstr="Unknown error", status=0,
                       filename=None, executor=None, action=None, command=None,
                       *args):
        self.node = node
        self.errstr = errstr
        self.status = status
        self.filename = filename
        self.executor = executor
        self.action = action
        self.command = command
        apply(Exception.__init__, (self,) + args)

class InternalError(Exception):
    pass

class UserError(Exception):
    pass

class StopError(Exception):
    pass

class EnvironmentError(Exception):
    pass

class ExplicitExit(Exception):
    def __init__(self, node=None, status=None, *args):
        self.node = node
        self.status = status
        apply(Exception.__init__, (self,) + args)

class TaskmasterException(Exception):
    def __init__(self, node=None, exc_info=(None, None, None), *args):
        self.node = node
        self.errstr = "Exception"
        self.exc_info = exc_info
        apply(Exception.__init__, (self,) + args)
