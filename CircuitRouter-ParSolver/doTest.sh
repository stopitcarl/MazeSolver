#!/bin/bash

threads=$1
resultFile=$2.speedups.cvs
seqTime=10
parTime=5
speedup=$(echo "scale=6; ${seqTime}/${parTime}" | bc)
echo Speedup = ${speedup}