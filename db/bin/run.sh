#!/bin/bash

ROOTDIR=`git rev-parse --show-toplevel`
CONTAINER=vl-mysql

docker start $CONTAINER