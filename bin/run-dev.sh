#!/bin/bash

ROOTDIR=`git rev-parse --show-toplevel`
CONTAINER=virtual-lab-dev-container

while [ "$(docker ps -aq -f name=${CONTAINER})" ]
do
    docker rm ${CONTAINER}
done
docker run --network=VLNetwork --name=${CONTAINER} -v "${ROOTDIR}:/home/user/vl" -it virtual-lab/dev
