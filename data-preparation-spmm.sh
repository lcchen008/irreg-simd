#!/bin/bash

#SBATCH -J DataPreparation 
#SBATCH -o DataPreparation.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p normal

SECONDS=0

#################### This script runs on CPU ######################

#################### Prepare data for SpMM including both float and double executions ######################

#################### This script is here in case you want to run SpMM only, and can save you time ######################

##################### For artificial evaluation, let's only test on clustered workloads, which are our work's focus #######################
cd spmm-float-and-double
cd datasets
echo -e "\e[0;32m **** Downloading input datasets for SpMM **** \e[0m"
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

echo -e "\e[0;32m **** Data preparation for SpMM finished! You can run programs now. Total time elapsed: $SECONDS secs ... **** \e[0m"
