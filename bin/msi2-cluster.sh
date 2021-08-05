#!/bin/bash

ROOTDIR=`git rev-parse --show-toplevel`
#${ROOTDIR}/bin/build-msi.sh

ssh -oStrictHostKeyChecking=no -R 172.0.0.1:3457:127.0.0.1:3457 -L 127.0.0.1:12342:127.0.0.1:12342 cn1162 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh 12342' &
ssh -oStrictHostKeyChecking=no -R 172.0.0.1:3457:127.0.0.1:3457 -L 127.0.0.1:12343:127.0.0.1:12343 cn1042 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh 12343' &
ssh -oStrictHostKeyChecking=no -R 172.0.0.1:3457:127.0.0.1:3457 -L 127.0.0.1:12344:127.0.0.1:12344 cn1139 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh 12344' &
ssh -oStrictHostKeyChecking=no -R 172.0.0.1:3457:127.0.0.1:3457 -L 127.0.0.1:12345:127.0.0.1:12345 cn1139 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh 12345' &
ssh -oStrictHostKeyChecking=no -R 172.0.0.1:3457:127.0.0.1:3457 -L 127.0.0.1:12346:127.0.0.1:12346 cn1130 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh 12346' &
ssh -oStrictHostKeyChecking=no -R 172.0.0.1:3457:127.0.0.1:3457 -L 127.0.0.1:12347:127.0.0.1:12347 cn1159 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh 12347' &
ssh -oStrictHostKeyChecking=no -R 172.0.0.1:3457:127.0.0.1:3457 -L 127.0.0.1:12348:127.0.0.1:12348 cn1045 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh 12348' &
