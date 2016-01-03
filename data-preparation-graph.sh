#!/bin/bash

#SBATCH -J DataPreparation 
#SBATCH -o DataPreparation.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p normal

SECONDS=0

#################### This script runs on CPU ######################

#################### Prepare data for graph applications, including PageRank and SSSP ######################
#################### This script is here in case you want to run graph applications only, and can save you time ######################

cd datasets
rm tiling
icc -O3 tiling.cpp -o tiling -std=c++11
cd pokec
wget https://snap.stanford.edu/data/soc-pokec-relationships.txt.gz
gunzip soc-pokec-relationships.txt.gz

echo 1632803 30622564 > soc-pokec
cat soc-pokec-relationships.txt >> soc-pokec

echo -e "\e[0;32m **** Tiling soc-pokec ... **** \e[0m"
../tiling soc-pokec

cd ../twitter
wget https://snap.stanford.edu/data/higgs-social_network.edgelist.gz
gunzip higgs-social_network.edgelist.gz
echo 456626 14855842 > higgs-twitter
cat higgs-social_network.edgelist >> higgs-twitter

echo -e "\e[0;32m **** Tiling higgs-twitter ... **** \e[0m"
../tiling higgs-twitter
cd ../..
# Now at outer most folder.

echo -e "\e[0;32m **** Data preparation for graph applications finished! You can run programs now. Total time elapsed: $SECONDS secs ... **** \e[0m"
