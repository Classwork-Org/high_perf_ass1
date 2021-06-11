#!/bin/bash

rm -rf results.csv;
rm -rf temp.*;

printf "NTHREADS\tARRAY_SIZE" | tee -a results.csv;
for (( i = 0; i < 10; i++ )); do
	printf "\tTIME_%s" ${i} | tee -a results.csv;
done
printf "\n" | tee -a results.csv;

for (( nthreads = 256; nthreads >= 1; nthreads/=2 )); do
	for (( arraySize = 100000; arraySize < 1000000000; arraySize*=10 )); do
		sed -e "s/\#define\ NTHREAD\ 8/\#define\ NTHREAD\ $nthreads/g" \
		-e "s/\#define\ TEST_ARRAY_SIZE\ 10000000/\#define\ TEST_ARRAY_SIZE\ $arraySize/g" qsort.c > temp.c;
		gcc -O3 ./temp.c -lpthread -o temp.out;
		
		printf "%s\t%s" ${nthreads} ${arraySize} | tee -a results.csv;
		for (( i = 0; i < 10; i++ )); do
			runtime=$((time ./temp.out) |& grep real | grep -oP "([0-9]+\.[0-9]+)");
			printf "\t%s" $runtime | tee -a results.csv
		done
		printf "\n"  | tee -a results.csv;
	done
done