#!/bin/bash
scp main mic0:/tmp
ssh mic0 LD_LIBRARY_PATH=/tmp /tmp/main /work/02687/binren/linchuan/spmm/datasets/msc10848/msc10848.mtx 1800000
