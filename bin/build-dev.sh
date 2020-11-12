#!/bin/bash

ROOTDIR=`git rev-parse --show-toplevel`
CONTAINER=virtual-lab-dev-container

docker build $1 \
	--target dev \
	--build-arg USER_ID=$(id -u) \
	--build-arg GROUP_ID=$(id -g) \
	-t virtual-lab/dev "$ROOTDIR"

while [ "$(docker ps -aq -f name=${CONTAINER})" ]
do
    docker rm ${CONTAINER}
done

