#!/bin/bash

PORT=$1

if [ -z "$1" ]
then
    PORT=8081
fi

ROOTDIR=`git rev-parse --show-toplevel`
CONTAINER=virtual-lab-dev-container

#while [ "$(docker ps -aq -f name=${CONTAINER})" ]
#do
#    docker rm ${CONTAINER}
#done
#docker run --rm --network=VLNetwork --name=${CONTAINER} -p 127.0.0.1:$PORT:$PORT -v "${ROOTDIR}:/home/user/vl" -it virtual-lab/dev
docker run --network=VLNetwork --rm --name=${CONTAINER} -p 127.0.0.1:$PORT:$PORT -v "${ROOTDIR}:/home/user/vl" -it virtual-lab/dev
#docker run --network=VLNetwork --name=${CONTAINER} -p 127.0.0.1:$PORT:$PORT -p 127.0.0.1:3457:3457 -v "${ROOTDIR}:/home/user/vl" -it virtual-lab/dev
# ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:1234:127.0.0.1:1234 dtorban@jimmypage.cs.umn.edu
# ssh -R 3457:dockerServerIP:3457 -L localhostIP:1234:127.0.0.1:1234 dtorban@jimmypage.cs.umn.edu

# ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:1234:127.0.0.1:1234 dtorban@jimmypage.cs.umn.edu
# ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12341:127.0.0.1:12341 dtorban@csel-kh1250-05.cselabs.umn.edu
