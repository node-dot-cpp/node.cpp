#!/bin/bash
f='...'
echo $#
if [ "$#" -ne 1 ]; then

	f='test.txt'
	echo  "Output file is ${f}"
    echo "To set other output name  start ./test.sh file_name.txt"
else
	f=$1
	echo  "Output file is ${f}"
fi

for ((i=1000; i<=12000; i=i+1000))
do
echo "for $i" >> $f
httperf --timeout=5 --client=0/1 --server=127.0.0.1 --port=2000  --rate=$i --send-buffer=4096 --recv-buffer=16384 --num-conns=25000 --num-calls=10 >> $f
sleep  100
done
