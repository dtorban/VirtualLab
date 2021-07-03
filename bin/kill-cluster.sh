#!/bin/bash


# ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:1234:127.0.0.1:1234 dtorban@jimmypage.cs.umn.edu
# ssh -R 3457:172.21.0.2:3457 -L 172.21.0.1:12341:127.0.0.1:12341 dtorban@csel-kh1250-05.cselabs.umn.edu

#while true
#do
#  echo Keep running
#  sleep 3
#done

#ps aux | grep /home/dtorban/CellModel | awk '{print $2}' | xargs -r kill -9
#ssh dtorban@csel-kh1250-05.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9"

ps aux | grep /home/dtorban/CellModel | awk '{print $2}' | xargs -r kill -9
ssh dtorban@jimmypage.cs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9"
ssh dtorban@csel-kh1250-05.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@csel-kh1250-06.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@csel-kh1250-07.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@csel-kh1250-08.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@csel-kh1250-09.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@csel-kh1250-10.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@csel-kh1250-11.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@csel-kh1250-12.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@csel-kh1250-13.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &
ssh dtorban@csel-kh1250-14.cselabs.umn.edu "ps aux | grep CellModel | awk '{print \$2}' | xargs -r kill -9" &