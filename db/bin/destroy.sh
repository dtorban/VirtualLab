#!/bin/bash

ROOTDIR=`git rev-parse --show-toplevel`
CONTAINER=vl-mysql

mkdir -p ${ROOTDIR}/db/storage/$CONTAINER

docker kill ${CONTAINER}
docker rm ${CONTAINER}

rm -rf ${ROOTDIR}/db/storage/$CONTAINER