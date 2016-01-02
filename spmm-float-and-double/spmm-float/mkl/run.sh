#ssh mic0 LD_LIBRARY_PATH=/tmp /tmp/main /work/02687/binren/linchuan/spmm/datasets/msc10848/msc10848.mtx 1800000
#ssh mic0 LD_LIBRARY_PATH=/tmp /tmp/main /work/02687/binren/linchuan/spmm/datasets/conf5_4-8x8-05/conf5_4-8x8-05.mtx 11000000 
#ssh mic0 LD_LIBRARY_PATH=/tmp /tmp/main /work/02687/binren/linchuan/spmm/datasets/shipsec5/shipsec5.mtx 14000000 
amplxe-cl -collect knc-general-exploration -knob enable-tlb-metrics=true -r "/work/02687/binren/linchuan/spmm/mkl/mkl_vtune" /tmp/main /work/02687/binren/linchuan/spmm/datasets/crankseg_1/crankseg_1.mtx 24000000 
