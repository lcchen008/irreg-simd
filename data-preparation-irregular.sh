#!/bin/bash

#SBATCH -J DataPreparation 
#SBATCH -o DataPreparation.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p normal

SECONDS=0

#################### This script runs on CPU ######################

#################### Prepare data for Irregular Reducsions, including Moldyn and Euler ######################
#################### This script is here in case you want to run irregular reductions only, and can save you time ######################

cd input/gen_data
make clean
make
cd ../..

# Generate original data for Moldyn. Two folders will be generated: 
#   1. input/32-3.0r
#      + 32-3.0r.mesh  --> The interaction list file
#      + 32-3.0r.xyz   --> The node data file
#   2. input/45-3.0r
#      + 45-3.0r.mesh  --> The interaction list file
#      + 45-3.0r.xyz   --> The node data file

echo -e "\e[0;32m **** Generating input for moldyn... **** \e[0m"
echo
echo -e "\e[0;32m **** Generating 32-3.0r ... **** \e[0m"
./input/gen_data/gen < gen-32-3.0
echo -e "\e[0;32m **** Generating 45-3.0r ... **** \e[0m"
./input/gen_data/gen < gen-45-3.0

# Transform the above data to matrix market format, which is used for
# tiling.
echo -e "\e[0;32m **** Transforming moldyn input to standard matrix market format... **** \e[0m"
cd input
g++ -O3 trans-mesh-to-mm.cpp -o tmtm --std=c++0x
echo -e "\e[0;32m **** Transforming 32-3.0r ... **** \e[0m"
./tmtm 32-3.0r/32-3.0r.mesh
echo -e "\e[0;32m **** Transforming 45-3.0r ... **** \e[0m"
./tmtm 45-3.0r/45-3.0r.mesh

################### Prepare data for Euler ######################

# Download the sparse matrices from UFL sparse matrix dataset website.
echo -e "\e[0;32m **** Downloading input kron_g500-logn19 for euler **** \e[0m"
wget http://www.cise.ufl.edu/research/sparse/MM/DIMACS10/kron_g500-logn19.tar.gz
tar -xvzf kron_g500-logn19.tar.gz
rm kron_g500-logn19.tar.gz

# Convert to mesh data.
echo -e "\e[0;32m **** Converting kron_g500-logn19 to mesh **** \e[0m"
./tran_mm_to_adj.py kron_g500-logn19/kron_g500-logn19.mtx
# Convert the above result to matrix market format.
echo -e "\e[0;32m **** Converting the above data to matrix market **** \e[0m"
./tmtm kron_g500-logn19/kron_g500-logn19.mtx.mesh

echo -e "\e[0;32m **** Downloading input gsm_106857 for euler **** \e[0m"
wget http://www.cise.ufl.edu/research/sparse/MM/Dziekonski/gsm_106857.tar.gz
tar -xvzf gsm_106857.tar.gz
rm gsm_106857.tar.gz

# Convert to mesh data.
echo -e "\e[0;32m **** Converting gsm_106857 to mesh **** \e[0m"
./tran_mm_to_adj.py gsm_106857/gsm_106857.mtx
# Convert the above result to matrix market format.
echo -e "\e[0;32m **** Converting the above data to matrix market **** \e[0m"
./tmtm gsm_106857/gsm_106857.mtx.mesh

################## Tile the data and remove conflicts #############
echo -e "\e[0;32m **** Tiling inputs of moldyn and euler **** \e[0m"
echo -e "\e[0;32m **** Be patient, this will take a long time **** \e[0m"
cd ..
cd tools
make clean
make
./tile-inputs.sh
cd ..
# Now at the outer most directory.

echo -e "\e[0;32m **** Data preparation for irregular reductions finished! You can run programs now. Total time elapsed: $SECONDS secs ... **** \e[0m"
