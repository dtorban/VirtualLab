#!/bin/bash

ROOTDIR=`git rev-parse --show-toplevel`

docker network create VLNetwork

${ROOTDIR}/db/bin/create.sh
${ROOTDIR}/bin/build-dev.sh