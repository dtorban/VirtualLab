#!/bin/bash

ps aux | grep msi-setup-worker.sh | awk '{print $2}' | xargs -r kill -9
ssh cn1162 "/home/keefedf/dtorban/project/VirtualLab/bin/msi-kill-worker.sh" &
ssh cn1042 "/home/keefedf/dtorban/project/VirtualLab/bin/msi-kill-worker.sh" &
ssh cn1139 "/home/keefedf/dtorban/project/VirtualLab/bin/msi-kill-worker.sh" &
ssh cn1139 "/home/keefedf/dtorban/project/VirtualLab/bin/msi-kill-worker.sh" &
ssh cn1130 "/home/keefedf/dtorban/project/VirtualLab/bin/msi-kill-worker.sh" &
ssh cn1159 "/home/keefedf/dtorban/project/VirtualLab/bin/msi-kill-worker.sh" &
ssh cn1045 "/home/keefedf/dtorban/project/VirtualLab/bin/msi-kill-worker.sh" &