#!/bin/bash

ps aux | grep msi-setup-worker.sh | awk '{print $2}' | xargs -r kill -9
ssh dtorban@ln1001.msi.umn.edu "ps aux | grep msi-setup-worker.sh | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@ln1001.msi.umn.edu ssh cn0073 "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@ln1001.msi.umn.edu ssh cn0074 "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
-