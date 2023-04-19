#!/bin/bash -x

#usage: ./run-bob.sh <testname> <alice_ip_address>
#e.g., ./run-bob.sh tpch_test "192.168.0.1"

if [ $# -lt 1 ]; 
then
   printf "usage:  ./run-bob.sh <testname>\n"
   printf "Not enough arguments - %d\n" $# 
   exit 0 
fi

./bin/$1 --party=2 --alice_host=$2
