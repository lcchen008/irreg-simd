#!/bin/bash

echo mimd on tile 1024
./page_rank ../../datasets/twitter/higgs-twitter-1024 1024
echo mimd on tile 2048
./page_rank ../../datasets/twitter/higgs-twitter-2048 2048
echo mimd on tile 4096
./page_rank ../../datasets/twitter/higgs-twitter-4096 4096
echo mimd on tile 8192
./page_rank ../../datasets/twitter/higgs-twitter-8192 8192

