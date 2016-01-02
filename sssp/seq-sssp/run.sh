#!/bin/bash

make
echo serial sssp on higgs-twitter
./sssp ../../datasets/twitter/higgs-twitter-orig
echo serial sssp on soc-pokec
./sssp ../../datasets/pokec/soc-pokec-orig
