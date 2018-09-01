#!/bin/bash
for i in `seq 1 10000`;
do
  nc -v -w 1 -z localhost 50001 &
done

