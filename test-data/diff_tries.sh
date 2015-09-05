#!/bin/sh

NUM_PARAMS=3
if [ "$#" -ne ${NUM_PARAMS} ]; then
   echo "Illegal number of parameters: $# expected ${NUM_PARAMS}"
   echo "1 - the platform: {Linux, MaxOS, Centos}"
   echo "2 - the model to run on"
   echo "3 - the query input file to use"
   exit 1
fi

echo "c2wa"
../dist/Release__${1}_/back-off-language-model-smt ${2} ${3} c2wa info3 | grep -v "CPU" | grep -v "vmsize" | grep -v "Cultivating" > release.c2wa.out
echo "c2dm"
../dist/Release__${1}_/back-off-language-model-smt ${2} ${3} c2dm info3  | grep -v "CPU" | grep -v "vmsize" | grep -v "Cultivating" > release.c2dm.out
echo "w2ca"
../dist/Release__${1}_/back-off-language-model-smt ${2} ${3} w2ca info3 | grep -v "CPU" | grep -v "vmsize" | grep -v "Cultivating" | grep -v "' memory allocation strategy." > release.w2ca.out
echo "w2ch"
../dist/Release__${1}_/back-off-language-model-smt ${2} ${3} w2ch info3 | grep -v "CPU" | grep -v "vmsize" | grep -v "Cultivating" > release.w2ch.out
echo "c2dh"
../dist/Release__${1}_/back-off-language-model-smt ${2} ${3} c2dh info3 | grep -v "CPU" | grep -v "vmsize" | grep -v "Cultivating" > release.c2dh.out

echo "\n----> c2wa vs. c2dm"
diff release.c2wa.out release.c2dm.out > diff.c2wa.c2dm.out
cat diff.c2wa.cmhm.out

echo "\n----> c2wa vs. w2ca"
diff release.c2wa.out release.w2ca.out > diff.c2wa.w2ca.out
cat diff.c2wa.w2ca.out

echo "\n----> c2wa vs. w2ch"
diff release.c2wa.out release.w2ch.out > diff.c2wa.w2ch.out
cat diff.c2wa.w2ch.out

echo "\n----> c2wa vs. c2dh"
diff release.c2wa.out release.c2dh.out > diff.c2wa.c2dh.out
cat diff.c2wa.c2dh.out
