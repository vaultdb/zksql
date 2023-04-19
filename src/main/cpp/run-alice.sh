#!/bin/bash -x

#usage: ./run-alice.sh <testname> <alice_ip_address>
#e.g., ./run-alice.sh tpch_test "192.168.0.1"

if [ $# -lt 1 ]; 
then
   printf "usage:  ./run-alice.sh <testname>\n"
   printf "Not enough arguments - %d\n" $# 
   exit 0 
fi

./bin/$1 --party=1 --alice_host=$2
