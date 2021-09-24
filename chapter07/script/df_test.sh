#!/bin/bash

df > log/old.log

mkdir temp
cd temp
for ((i = 0; i < 10000; ++i)); do mkdir $i; done
cd ..

df > log/new.log


df | head -1
diff log/old.log log/new.log

rm -r temp
rm log/*
