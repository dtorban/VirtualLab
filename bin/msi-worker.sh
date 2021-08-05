#!/bin/bash

ssh -R 172.21.0.2:3457:127.0.0.1:3457 -L 127.0.0.1:12342:127.0.0.1:12342 dtorban@cn0073 '/home/keefedf/dtorban/project/VirtualLab/bin/msi-setup-worker.sh'
