#/bin/bash

sh ../../source/killMProcess.sh
cd ..
make
cd samples
sh compile.sh | sh run.sh
