"""SCons.Tool.tex

Tool-specific initialization for TeX.

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

__revision__ = "src/engine/SCons/Tool/tex.py 3266 2008/08/12 07:31:01 knight"

import os.path
import re
import string

import SCons.Action
import SCons.Node
import SCons.Node.FS
import SCons.Util

warning_rerun_re = re.compile('(^LaTeX Warning:.*Rerun)|(^Package \w+ Warning:.*Rerun)', re.MULTILINE)

rerun_citations_str = "^LaTeX Warning:.*\n.*Rerun to get citations correct"
rerun_citations_re = re.compile(rerun_citations_str, re.MULTILINE)

undefined_references_str = '(^LaTeX Warning:.*undefined references)|(^Package \w+ Warning:.*undefined citations)'
undefined_references_re = re.compile(undefined_references_str, re.MULTILINE)

openout_aux_re = re.compile(r"\\openout.*`(.*\.aux)'")
openout_re = re.compile(r"\\openout.*`(.*)'")

makeindex_re = re.compile(r"^[^%]*\\makeindex", re.MULTILINE)
tableofcontents_re = re.compile(r"^[^%]*\\tableofcontents", re.MULTILINE)
bibliography_re = re.compile(r"^[^%]*\\bibliography", re.MULTILINE)

# An Action sufficient to build any generic tex file.
TeXAction = None

# An action to build a latex file.  This action might be needed more
# than once if we are dealing with labels and bibtex.
LaTeXAction = None

# An action to run BibTeX on a file.
BibTeXAction = None

# An action to run MakeIndex on a file.
MakeIndexAction = None

def InternalLaTeXAuxAction(XXXLaTeXAction, target = None, source= None, env=None):
    """A builder for LaTeX files that checks the output in the aux file
    and decides how many times to use LaTeXAction, and BibTeXAction."""

    basename = SCons.Util.splitext(str(source[0]))[0]
    basedir = os.path.split(str(source[0]))[0]
    basefile = os.path.split(str(basename))[1]
    abspath = os.path.abspath(basedir)
    targetbase = SCons.Util.splitext(str(target[0]))[0]
    targetdir = os.path.split(str(target[0]))[0]

    # Not sure if these environment changes should go here or make the
    # user do them I undo all but TEXPICTS but there is still the side
    # effect of creating the empty (':') entries in the environment.

    def modify_env_var(env, var, abspath):
        try:
            save = env['ENV'][var]
        except KeyError:
            save = ':'
            env['ENV'][var] = ''
        if SCons.Util.is_List(env['ENV'][var]):
            env['ENV'][var] = [abspath] + env['ENV'][var]
        else:
            env['ENV'][var] = abspath + os.pathsep + env['ENV'][var]
        return save

    texinputs_save = modify_env_var(env, 'TEXINPUTS', abspath)
    bibinputs_save = modify_env_var(env, 'BIBINPUTS', abspath)
    bstinputs_save = modify_env_var(env, 'BSTINPUTS', abspath)
    texpicts_save = modify_env_var(env, 'TEXPICTS', abspath)

    # Create these file names with the target directory since they will
    # be made there.   That's because the *COM variables have the cd
    # command in the prolog.

    bblfilename = os.path.join(targetdir, basefile + '.bbl')
    bblContents = ""
    if os.path.exists(bblfilename):
        bblContents = open(bblfilename, "rb").read()

    idxfilename = os.path.join(targetdir, basefile + '.idx')
    idxContents = ""
    if os.path.exists(idxfilename):
        idxContents = open(idxfilename, "rb").read()

    tocfilename = os.path.join(targetdir, basefile + '.toc')
    tocContents = ""
    if os.path.exists(tocfilename):
        tocContents = open(tocfilename, "rb").read()

    # Run LaTeX once to generate a new aux file and log file.
    XXXLaTeXAction(target, source, env)

    # Decide if various things need to be run, or run again.  We check
    # for the existence of files before opening them--even ones like the
    # aux file that TeX always creates--to make it possible to write tests
    # with stubs that don't necessarily generate all of the same files.

    # Read the log file to find all .aux files
    logfilename = os.path.join(targetbase + '.log')
    auxfiles = []
    if os.path.exists(logfilename):
        content = open(logfilename, "rb").read()
        auxfiles = openout_aux_re.findall(content)

    # Now decide if bibtex will need to be run.
    for auxfilename in auxfiles:
        target_aux = os.path.join(targetdir, auxfilename)
        if os.path.exists(target_aux):
            content = open(target_aux, "rb").read()
            if string.find(content, "bibdata") != -1:
                bibfile = env.fs.File(targetbase)
                BibTeXAction(bibfile, bibfile, env)
                break

    must_rerun_latex = 0
    # Now decide if latex will need to be run again due to table of contents.
    if os.path.exists(tocfilename) and tocContents != open(tocfilename, "rb").read():
        must_rerun_latex = 1

    # Now decide if latex will need to be run again due to bibliography.
    if os.path.exists(bblfilename) and bblContents != open(bblfilename, "rb").read():
        must_rerun_latex = 1

    # Now decide if latex will need to be run again due to index.
    if os.path.exists(idxfilename) and idxContents != open(idxfilename, "rb").read():
        # We must run makeindex
        idxfile = env.fs.File(targetbase)
        MakeIndexAction(idxfile, idxfile, env)
        must_rerun_latex = 1

    if must_rerun_latex == 1:
        XXXLaTeXAction(target, source, env)

    # Now decide if latex needs to be run yet again to resolve warnings.
    logfilename = targetbase + '.log'
    for _ in range(int(env.subst('$LATEXRETRIES'))):
        if not os.path.exists(logfilename):
            break
        content = open(logfilename, "rb").read()
        if not warning_rerun_re.search(content) and \
           not rerun_citations_re.search(content) and \
           not undefined_references_re.search(content):
            break
        XXXLaTeXAction(target, source, env)

    env['ENV']['TEXINPUTS'] = texinputs_save
    env['ENV']['BIBINPUTS'] = bibinputs_save
    env['ENV']['BSTINPUTS'] = bibinputs_save

    # The TEXPICTS enviroment variable is needed by a dvi -> pdf step
    # later on Mac OSX so leave it,
    # env['ENV']['TEXPICTS']  = texpicts_save

    return 0

def LaTeXAuxAction(target = None, source= None, env=None):
    InternalLaTeXAuxAction( LaTeXAction, target, source, env )

LaTeX_re = re.compile("\\\\document(style|class)")

def is_LaTeX(flist):
    # Scan a file list to decide if it's TeX- or LaTeX-flavored.
    for f in flist:
        content = f.get_contents()
        if LaTeX_re.search(content):
            return 1
    return 0

def TeXLaTeXFunction(target = None, source= None, env=None):
    """A builder for TeX and LaTeX that scans the source file to
    decide the "flavor" of the source and then executes the appropriate
    program."""
    if is_LaTeX(source):
        LaTeXAuxAction(target,source,env)
    else:
        TeXAction(target,source,env)
    return 0

def tex_emitter(target, source, env):
    base = SCons.Util.splitext(str(source[0]))[0]
    targetbase = SCons.Util.splitext(str(target[0]))[0]

    target.append(targetbase + '.aux')
    env.Precious(targetbase + '.aux')
    target.append(targetbase + '.log')
    for f in source:
        content = f.get_contents()
        if tableofcontents_re.search(content):
            target.append(targetbase + '.toc')
            env.Precious(targetbase + '.toc')
        if makeindex_re.search(content):
            target.append(targetbase + '.ilg')
            target.append(targetbase + '.ind')
            target.append(targetbase + '.idx')
            env.Precious(targetbase + '.idx')
        if bibliography_re.search(content):
            target.append(targetbase + '.bbl')
            env.Precious(targetbase + '.bbl')
            target.append(targetbase + '.blg')

    # read log file to get all .aux files
    logfilename = targetbase + '.log'
    dir, base_nodir = os.path.split(targetbase)
    if os.path.exists(logfilename):
        content = open(logfilename, "rb").read()
        out_files = openout_re.findall(content)
        out_files = filter(lambda f, b=base_nodir+'.aux': f != b, out_files)
        if dir != '':
            out_files = map(lambda f, d=dir: d+os.sep+f, out_files)
        target.extend(out_files)
        for f in out_files:
            env.Precious( f )

    return (target, source)

TeXLaTeXAction = None

def generate(env):
    """Add Builders and construction variables for TeX to an Environment."""

    # A generic tex file Action, sufficient for all tex files.
    global TeXAction
    if TeXAction is None:
        TeXAction = SCons.Action.Action("$TEXCOM", "$TEXCOMSTR")

    # An Action to build a latex file.  This might be needed more
    # than once if we are dealing with labels and bibtex.
    global LaTeXAction
    if LaTeXAction is None:
        LaTeXAction = SCons.Action.Action("$LATEXCOM", "$LATEXCOMSTR")

    # Define an action to run BibTeX on a file.
    global BibTeXAction
    if BibTeXAction is None:
        BibTeXAction = SCons.Action.Action("$BIBTEXCOM", "$BIBTEXCOMSTR")

    # Define an action to run MakeIndex on a file.
    global MakeIndexAction
    if MakeIndexAction is None:
        MakeIndexAction = SCons.Action.Action("$MAKEINDEXCOM", "$MAKEINDEXCOMSTR")

    global TeXLaTeXAction
    if TeXLaTeXAction is None:
        TeXLaTeXAction = SCons.Action.Action(TeXLaTeXFunction, strfunction=None)

    import dvi
    dvi.generate(env)

    bld = env['BUILDERS']['DVI']
    bld.add_action('.tex', TeXLaTeXAction)
    bld.add_emitter('.tex', tex_emitter)

    env['TEX']      = 'tex'
    env['TEXFLAGS'] = SCons.Util.CLVar('')
    env['TEXCOM']   = 'cd ${TARGET.dir} && $TEX $TEXFLAGS ${SOURCE.file}'

    # Duplicate from latex.py.  If latex.py goes away, then this is still OK.
    env['LATEX']        = 'latex'
    env['LATEXFLAGS']   = SCons.Util.CLVar('')
    env['LATEXCOM']     = 'cd ${TARGET.dir} && $LATEX $LATEXFLAGS ${SOURCE.file}'
    env['LATEXRETRIES'] = 3

    env['BIBTEX']      = 'bibtex'
    env['BIBTEXFLAGS'] = SCons.Util.CLVar('')
    env['BIBTEXCOM']   = 'cd ${TARGET.dir} && $BIBTEX $BIBTEXFLAGS ${SOURCE.filebase}'

    env['MAKEINDEX']      = 'makeindex'
    env['MAKEINDEXFLAGS'] = SCons.Util.CLVar('')
    env['MAKEINDEXCOM']   = 'cd ${TARGET.dir} && $MAKEINDEX $MAKEINDEXFLAGS ${SOURCE.file}'

def exists(env):
    return env.Detect('tex')
