#!/bin/bash

ROOTDIR=`git rev-parse --show-toplevel`
#${ROOTDIR}/bin/build-msi.sh

servers=`cat ./servers.txt`

arr=($servers)
port=12342

for i in ${arr[@]}; do
	ssh -oStrictHostKeyChecking=no $i "/home/keefedf/dtorban/project/VirtualLab/bin/msi-kill-worker.sh" & 
	port=$((port+1))
done

#ssh -oStrictHostKeyChecking=no -R 172.0.0.1:3457:127.0.0.1:3457 -L 127.0.0.1:12342:127.0.0.1:12342 cn1162 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh 12342' &
