#!/bin/bash

#SBATCH -J DataPreparation 
#SBATCH -o DataPreparation.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p normal

SECONDS=0

#################### This script runs on CPU ######################

#################### Prepare data for Moldyn ######################

cd input/gen_data
make clean
make
cd ..

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
./gen_data/gen < gen-32-3.0
echo -e "\e[0;32m **** Generating 45-3.0r ... **** \e[0m"
./gen_data/gen < gen-45-3.0
cd ..
# Now at out most folder.

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

##################### Prepare data for SpMM ########################
##################### For artificial evaluation, let's only test on clustered workloads, which are our work's focus #######################
echo -e "\e[0;32m **** Downloading input datasets for SpMM **** \e[0m"
cd ..
cd spmm-float-and-double
cd datasets
wget http://www.cise.ufl.edu/research/sparse/MM/Boeing/msc10848.tar.gz
wget http://www.cise.ufl.edu/research/sparse/MM/QCD/conf5_4-8x8-05.tar.gz
wget http://www.cise.ufl.edu/research/sparse/MM/DNVS/shipsec5.tar.gz
wget http://www.cise.ufl.edu/research/sparse/MM/GHS_psdef/crankseg_1.tar.gz
tar -xvzf msc10848.tar.gz
rm msc10848.tar.gz
tar -xvzf conf5_4-8x8-05.tar.gz
rm conf5_4-8x8-05.tar.gz
tar -xvzf shipsec5.tar.gz
rm shipsec5.tar.gz
tar -xvzf crankseg_1.tar.gz
rm crankseg_1.tar.gz

cd ../..
# Now at the outer most directory.


##################### Prepare data for Graph Applications ########################

cd datasets
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

echo -e "\e[0;32m **** Data preparation finished! You can run programs now. Total time elapsed: $SECONDS secs ... **** \e[0m"
