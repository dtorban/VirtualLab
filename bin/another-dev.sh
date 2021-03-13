#!/bin/bash

PORT=$1

if [ -z "$1" ]
then
    PORT=8081
fi

ROOTDIR=`git rev-parse --show-toplevel`
CONTAINER=virtual-lab-dev-container2

while [ "$(docker ps -aq -f name=${CONTAINER})" ]
do
    docker rm ${CONTAINER}
done
docker run --network=VLNetwork --name=${CONTAINER} -v "${ROOTDIR}:/home/user/vl" -it virtual-lab/dev
