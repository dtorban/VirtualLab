#!/bin/bash

ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12342:127.0.0.1:12342 dtorban@csel-kh1250-05.cselabs.umn.edu 'cd /export/scratch/dtorban/VirtualLab; git pull; git checkout experiment; make -j; cp build/bin/CellModel /home/dtorban/CellModel'
