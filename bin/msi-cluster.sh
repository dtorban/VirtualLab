#!/bin/bash

ROOTDIR=`git rev-parse --show-toplevel`
#${ROOTDIR}/bin/build-msi.sh

ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12342:127.0.0.1:12342 dtorban@ln1002.msi.umn.edu ssh -R 172.21.0.2:3457:127.0.0.1:3457 -L 127.0.0.1:12342:127.0.0.1:12342 cn1159 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh 12342' &
#ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12343:127.0.0.1:12343 dtorban@ln1001.msi.umn.edu ssh -R 172.21.0.2:3457:127.0.0.1:3457 -L 127.0.0.1:12343:127.0.0.1:12343 cn0074 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh 12343' &