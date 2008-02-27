#!/bin/bash

list='ls *.cpp *.h'
for i in $list
do
  expand --tabs=2 $i > temp 
  cp temp $i 
done
rm ls temp
