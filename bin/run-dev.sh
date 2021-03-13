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
docker run --rm --network=VLNetwork --name=${CONTAINER} -p 127.0.0.1:$PORT:$PORT -v "${ROOTDIR}:/home/user/vl" -it virtual-lab/dev
#docker run --network=VLNetwork --name=${CONTAINER} -p 127.0.0.1:$PORT:$PORT -p 127.0.0.1:3457:3457 -v "${ROOTDIR}:/home/user/vl" -it virtual-lab/dev
