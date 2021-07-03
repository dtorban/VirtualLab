#!/bin/bash

ROOTDIR=`git rev-parse --show-toplevel`
${ROOTDIR}/bin/build-cse.sh

#ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12342:127.0.0.1:12342 dtorban@csel-kh1250-05.cselabs.umn.edu 'cd /export/scratch/dtorban/VirtualLab; git pull; make -j; cp CellModel /home/dtorban/CellModel'

#ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12342:127.0.0.1:12342 dtorban@csel-kh1250-05.cselabs.umn.edu '/home/dtorban/CellModel 12342' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12341:127.0.0.1:12341 dtorban@jimmypage.cs.umn.edu '/home/dtorban/CellModel 12341' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12342:127.0.0.1:12342 dtorban@csel-kh1250-05.cselabs.umn.edu '/home/dtorban/CellModel 12342' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12343:127.0.0.1:12343 dtorban@csel-kh1250-06.cselabs.umn.edu '/home/dtorban/CellModel 12343' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12344:127.0.0.1:12344 dtorban@csel-kh1250-07.cselabs.umn.edu '/home/dtorban/CellModel 12344' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12345:127.0.0.1:12345 dtorban@csel-kh1250-08.cselabs.umn.edu '/home/dtorban/CellModel 12345' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12346:127.0.0.1:12346 dtorban@csel-kh1250-09.cselabs.umn.edu '/home/dtorban/CellModel 12346' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12347:127.0.0.1:12347 dtorban@csel-kh1250-10.cselabs.umn.edu '/home/dtorban/CellModel 12347' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12348:127.0.0.1:12348 dtorban@csel-kh1250-11.cselabs.umn.edu '/home/dtorban/CellModel 12348' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12349:127.0.0.1:12349 dtorban@csel-kh1250-12.cselabs.umn.edu '/home/dtorban/CellModel 12349' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12350:127.0.0.1:12350 dtorban@csel-kh1250-13.cselabs.umn.edu '/home/dtorban/CellModel 12350' &
ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12351:127.0.0.1:12351 dtorban@csel-kh1250-14.cselabs.umn.edu '/home/dtorban/CellModel 12351' &

# ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:1234:127.0.0.1:1234 dtorban@jimmypage.cs.umn.edu
# ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12341:127.0.0.1:12341 dtorban@csel-kh1250-05.cselabs.umn.edu

#while true
#do
#  echo Keep running
#  sleep 3
#done

#ps aux | grep /home/dtorban/CellModel | awk '{print $2}' | xargs -r kill -9
#ssh dtorban@csel-kh1250-05.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9"
