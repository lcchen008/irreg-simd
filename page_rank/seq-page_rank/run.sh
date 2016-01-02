#!/bin/bash

make
echo serial page_rank on higgs-twitter
./page_rank ../../datasets/twitter/higgs-twitter-orig
echo serial page_rank on soc-pokec
./page_rank ../../datasets/pokec/soc-pokec-orig
