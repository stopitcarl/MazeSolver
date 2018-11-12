#!/bin/bash

if [ $# -ne 2 ]
  then
    echo "Sufficient arguments not provided"		
	exit;
fi

threads=$1
resultFile=$2.speedups.cvs
seqTime=0

echo $resultFile

echo "#threads, exec_time, speedup" > $resultFile

../CircuitRouter-SeqSolver/./CircuitRouter-SeqSolver $2
seqTime=$(cat ../CircuitRouter-SeqSolver/$2.res | grep time | cut -c19-26)
echo seqTime = $seqTime >> $resultFile
for (( i=1; i<=$threads; i++))
	do
		./CircuitRouter-ParSolver -t $i $2
		parTime=$(cat $2.res | grep time | cut -c19-26)
		speedup=$(echo "scale=6; $seqTime/$parTime" | bc)
		echo $i, $parTime, $speedup >> $resultFile
	done



