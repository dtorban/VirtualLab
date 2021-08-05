#!/bin/sh

#module load gcc/7.2.0
export LD_LIBRARY_PATH=/panfs/roc/msisoft/isl/0.18_gcc7.2.0/lib:/panfs/roc/msisoft/mpc/1.0.3_gcc7.2.0/lib:/panfs/roc/msisoft/mpfr/3.1.6_gcc7.2.0/lib:/panfs/roc/msisoft/gmp/6.1.2_gcc7.2.0/lib:/panfs/roc/msisoft/gcc/7.2.0/lib64
/home/keefedf/dtorban/project/VirtualLab/build/bin/CellModel $1
