#!/bin/bash

count=1

for (( i=0;i<=100;i++ ))
do
	pidstat -p `pidof a.out` -tr | tee -a memlog.txt;
	sleep 1;
done

