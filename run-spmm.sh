#!/bin/bash

#SBATCH -J SpMM 
#SBATCH -o SpMM.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p normal-mic

SECONDS=0

################## Executions ###################
  # Build binaries:
  cd spmm-float-and-double
  echo -e "\e[0;32m **** Building binaries... **** \e[0m"
  cd spmm-float/seq
  make clean
  make
  cd ../simd-naive
  make clean
  make
  cd ../simd-tiling
  make clean
  make
  cd ../mkl
  make clean
  make
  cd ../..
  cd spmm-double
  cd seq
  make clean
  make
  cd ../simd-naive
  make clean
  make
  cd ../simd-tiling
  make clean
  make
  cd ../mkl
  make clean
  make
# copy the mkl libraries to mic0.
  ./copy.sh
  cd ../..
# Now at spmm-float-and-double 

# Run SpMM with float execution
  cd spmm-float
  echo -e "\e[0;34m ************************ Running SpMM with float execution ************************** \e[0m"

  # Run on data msc10848
  echo -e "\e[0;34m ************************ Running SpMM with float execution on data msc10848 ************************** \e[0m"
  # Serial version, base line:
  echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
  cd seq
  ./run-msc.sh
  # SIMD Naive version:
  echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
  cd ../simd-naive
  ./run-msc.sh
  # SIMD TIling (Our) version:
  echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
  cd ../simd-tiling
  ./run-msc.sh
  # MKL version:
  echo -e "\e[0;32m **** Running [MKL] **** \e[0m"
  cd ../mkl
  ./run-msc.sh

  # Run on data conf5_4-8x8-05
  echo -e "\e[0;34m ************************ Running SpMM with float execution on data conf5_4-8x8-05 ************************** \e[0m"
  # Serial version, base line:
  echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
  cd ../seq
  ./run-conf.sh
  # SIMD Naive version:
  echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
  cd ../simd-naive
  ./run-conf.sh
  # SIMD TIling (Our) version:
  echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
  cd ../simd-tiling
  ./run-conf.sh
  # MKL version:
  echo -e "\e[0;32m **** Running [MKL] **** \e[0m"
  cd ../mkl
  ./run-conf.sh

  # Run on data shipsec5
  echo -e "\e[0;34m ************************ Running SpMM with float execution on data shipsec5 ************************** \e[0m"
  # Serial version, base line:
  echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
  cd ../seq
  ./run-ship.sh
  # SIMD Naive version:
  echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
  cd ../simd-naive
  ./run-ship.sh
  # SIMD TIling (Our) version:
  echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
  cd ../simd-tiling
  ./run-ship.sh
  # MKL version:
  echo -e "\e[0;32m **** Running [MKL] **** \e[0m"
  cd ../mkl
  ./run-ship.sh

  # Run on data crankseg_1
  echo -e "\e[0;34m ************************ Running SpMM with float execution on data crankseg_1 ************************** \e[0m"
  # Serial version, base line:
  echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
  cd ../seq
  ./run-crank.sh
  # SIMD Naive version:
  echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
  cd ../simd-naive
  ./run-crank.sh
  # SIMD TIling (Our) version:
  echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
  cd ../simd-tiling
  ./run-crank.sh
  # MKL version:
  echo -e "\e[0;32m **** Running [MKL] **** \e[0m"
  cd ../mkl
  ./run-crank.sh

  cd ../..
  # Now at spmm-float-and-double

# Run SpMM with double execution
  cd spmm-double
  echo -e "\e[0;34m ************************ Running SpMM with double execution ************************** \e[0m"
  # Run on data msc10848
  echo -e "\e[0;34m ************************ Running SpMM with double execution on data msc10848 ************************** \e[0m"
  # Serial version, base line:
  echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
  cd seq
  ./run-msc.sh
  # SIMD Naive version:
  echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
  cd ../simd-naive
  ./run-msc.sh
  # SIMD TIling (Our) version:
  echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
  cd ../simd-tiling
  ./run-msc.sh
  # MKL version:
  echo -e "\e[0;32m **** Running [MKL] **** \e[0m"
  cd ../mkl
  ./run-msc.sh


  # Run on data conf5_4-8x8-05
  echo -e "\e[0;34m ************************ Running SpMM with double execution on data conf5_4-8x8-05 ************************** \e[0m"
  # Serial version, base line:
  echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
  cd ../seq
  ./run-conf.sh
  # SIMD Naive version:
  echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
  cd ../simd-naive
  ./run-conf.sh
  # SIMD TIling (Our) version:
  echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
  cd ../simd-tiling
  ./run-conf.sh
  # MKL version:
  echo -e "\e[0;32m **** Running [MKL] **** \e[0m"
  cd ../mkl
  ./run-conf.sh


  # Run on data shipsec5
  echo -e "\e[0;34m ************************ Running SpMM with double execution on data shipsec5 ************************** \e[0m"
  # Serial version, base line:
  echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
  cd ../seq
  ./run-ship.sh
  # SIMD Naive version:
  echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
  cd ../simd-naive
  ./run-ship.sh
  # SIMD TIling (Our) version:
  echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
  cd ../simd-tiling
  ./run-ship.sh
  # MKL version:
  echo -e "\e[0;32m **** Running [MKL] **** \e[0m"
  cd ../mkl
  ./run-ship.sh

  # Run on data crankseg_1
  echo -e "\e[0;34m ************************ Running SpMM with double execution on data crankseg_1 ************************** \e[0m"
  # Serial version, base line:
  echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
  cd ../seq
  ./run-crank.sh
  # SIMD Naive version:
  echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
  cd ../simd-naive
  ./run-crank.sh
  # SIMD TIling (Our) version:
  echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
  cd ../simd-tiling
  ./run-crank.sh
  # MKL version:
  echo -e "\e[0;32m **** Running [MKL] **** \e[0m"
  cd ../mkl
  ./run-crank.sh

  cd ../../..
  # Now at the outer most directory. 

echo -e "\e[0;32m **** SpMM finished! Total time elapsed: $SECONDS secs ... **** \e[0m"
