#!/bin/bash

echo mimd sssp on tile 1024
./sssp ../../datasets/pokec/soc-pokec-1024 1024
echo mimd sssp on tile 2048
./sssp ../../datasets/pokec/soc-pokec-2048 2048
echo mimd sssp on tile 4096
./sssp ../../datasets/pokec/soc-pokec-4096 4096
echo mimd sssp on tile 8192
./sssp ../../datasets/pokec/soc-pokec-8192 8192
