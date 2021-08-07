#!/bin/bash

ps aux | grep msi-setup-worker.sh | awk '{print $2}' | xargs -r kill -9
ssh dtorban@ln1002.msi.umn.edu "ps aux | grep msi-setup-worker.sh | awk '{print \$2}' | xargs -r kill -9" &
#ssh dtorban@ln1002.msi.umn.edu ssh cn1068 "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@ln1002.msi.umn.edu ssh cn1159 "/home/keefedf/dtorban/project/VirtualLab/bin/msi-kill-worker.sh" &
#ssh dtorban@ln1001.msi.umn.edu ssh cn0074 "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
