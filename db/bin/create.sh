#!/bin/bash

ROOTDIR=`git rev-parse --show-toplevel`
CONTAINER=my-mysql

mkdir -p "${ROOTDIR}"/db/storage/$CONTAINER

docker build -t virtual-lab-db \
	--build-arg USER_ID=$(id -u) \
	--build-arg GROUP_ID=$(id -g) \
    "${ROOTDIR}"/db 

docker run -d -p 3306:3306 --name $CONTAINER \
    -v "${ROOTDIR}"/db/storage/$CONTAINER:/var/lib/mysql \
    -v "${ROOTDIR}"/db/sql-scripts:/docker-entrypoint-initdb.d/ \
    -e MYSQL_ROOT_PASSWORD=supersecret \
    -e MYSQL_DATABASE=company \
    virtual-lab-db