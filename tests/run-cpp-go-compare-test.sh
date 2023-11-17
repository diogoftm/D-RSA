#!/bin/bash
cd ../cpp
make
output1=$(./RBG pass confusion 10 --limit 100)


cd ../go
go mod tidy

cd generator
go build generator.go
output2=$(./generator pass confusion 10 --limit 100)

if [ "$output1" = "$output2" ]; then
    echo "C++ and Go generator produce equal output"
    exit 0
else
    exit 1
fi
