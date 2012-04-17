#!/bin/sh
export PATH=$PATH:`cwd`
mkdir transifex
cd transifex
tx init
tx set --auto-remote http://www.transifex.net/projects/p/ariamaestosa/
tx pull --all