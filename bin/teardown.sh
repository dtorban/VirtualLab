#!/bin/bash


ROOTDIR=`git rev-parse --show-toplevel`

${ROOTDIR}/db/bin/destroy.sh

docker network rm VLNetwork