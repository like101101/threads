#!/bin/bash

LOOP_COUNT=10
ITERATIONS=200000

make clean
make

if [[ ! -f "results.csv" ]];
then
	echo "i, latency, iterations, apporach" > results.csv
fi

function RunApproach1 {

	printf "Approach 1: Independent CPU work client\n"

	i=0
	while [ $i -lt $LOOP_COUNT ]
	do
    	IC=$(taskset -c 0 ./independent_client $ITERATIONS)
		sleep 0.2

    	i=$(( $i + 1 ))
		echo "${i}, ${IC}, ${ITERATIONS}, independent client" >> results.csv
		echo -n -e '  Completed Iterations: '"$i/$LOOP_COUNT"'\r'
	done
	printf "\n\n"
}


function RunApproach2 {

	printf "Approach 2: CPU work client + Ping-Pong\n"

	i=0
	while [ $i -lt $LOOP_COUNT ]
	do
    	taskset -c 0 ./pingpong_server $ITERATIONS &

		sleep 0.5
		PPSC=$(taskset -c 1 ./pingpong_client $ITERATIONS)

		wait
    	i=$(( $i + 1 ))
		echo "${i}, ${PPSC}, ${ITERATIONS}, client_work_pingpong" >> results.csv
		echo -n -e '  Completed Iterations: '"$i/$LOOP_COUNT"'\r'
	done
	printf "\n"
}


function RunApproach3 {

	printf "Approach 3: Client + CPU work Server\n"

	i=0
	while [ $i -lt $LOOP_COUNT ]
	do
    	taskset -c 0 ./server $ITERATIONS &

		sleep 0.5
		SC=$(taskset -c 1 ./ipc_client $ITERATIONS)

		wait
    	i=$(( $i + 1 ))
		echo "${i}, ${SC}, ${ITERATIONS}, server_client" >> results.csv
		echo -n -e '  Completed Iterations: '"$i/$LOOP_COUNT"'\r'
	done
	printf "\n"
}

printf "Choose one of the four approaches to IPC to test\n"
printf "\t[1] Independent CPU work client\n"
printf "\t[2] CPU work client + PingPong\n"
printf "\t[3] Client + Server\n"

read -p "Select your choice: " test_choice

if [ $test_choice == "1" ]; then
	RunApproach1
elif [ $test_choice == "2" ]; then
    RunApproach2
elif [ $test_choice == "3" ]; then
    RunApproach3
fi
