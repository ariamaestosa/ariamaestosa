This project should be helpful to build Aria Maestosa,
on both Linux and Mac OS X, as well as for editing code and debugging it.You will still need to build libjdkmidi on the terminal.Warning: by default, Code::Blocks launches commands using '/bin/sh -c',
which does NOT import your environment variables - so if you changed PATH
to get the right wx-config you may get unexpected results. Go in the
Code::Blocks settings and change the command to be '/bin/sh -l -c' instead.