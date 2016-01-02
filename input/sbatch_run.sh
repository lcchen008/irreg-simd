#!/bin/bash

#SBATCH -J reorder 
#SBATCH -o output_reorder.%j
#SBATCH -t 10:00:00
#SBATCH -n1 -N1
#SBATCH -p largemem

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

#ibrun ./rmd 1048576 64-1.0r/64-1.0r.mesh.matlab 64-1.0r/64-1.0r.mesh.metis.part.2048.ordering
ibrun ./tdu 64-1.0r/64-1.5r.mesh
#ibrun ./gof 2048 64-1.0r/64-1.5r.mesh.metis.part.2048
#ibrun ./rmd 1048576  64-1.0r/64-1.5r.mesh.matlab 64-1.0r/64-1.5r.mesh.metis.part.2048.ordering
#ibrun ./rc 64-1.0r/64-1.5r.xyz 64-1.0r/64-1.5r.mesh.metis.part.2048.ordering
#ibrun ./reorder.py pokec/soc-pokec-relationships.list
# ibrun ./reorder.py euler/mesh euler/mesh.xyz
#./reorder.py kron_g500-logn19/kron_g500-logn19.mtx.mesh kron_g500-logn19/kron_g500-logn19.mtx.xyz 
#./reorder.py gsm_106857/gsm_106857.mtx.mesh gsm_106857/gsm_106857.mtx.xyz 
#./reorder.py twitter/higgs-social_network.adj 
#./reorder.py 45-3.0r/45-3.0r.mesh 45-3.0r/45-3.0r.xyz
#./reorder.py 45-3.0r/45-3.0r.mesh 45-3.0r/45-3.0r.xyz
