# VirtualLab

## Pre-requisites
  * [Git](https://git-scm.com/)

## Docker Pre-requisites
  * Windows 10 Home
    * Install [wsl2 and Ubuntu](https://www.youtube.com/watch?v=ilKQHAFeQR0&list=RDCMUCzLbHrU7U3cUDNQWWAqjceA&start_radio=1&t=7)
  * Install [Docker Desktop](https://hub.docker.com/?overlay=onboarding) from [Docker Hub](https://hub.docker.com/)
  * Linux
    * Use [docker group instead of sudo](https://www.digitalocean.com/community/tutorials/how-to-install-and-use-docker-on-ubuntu-18-04)

## Clone Virtual Lab repository

1. Clone your personal repository of VirtualLab. <clone-ref> can be found in the Virtual Lab git hub code page. Then, go to the VirtualLab directory using 'cd'

    ```bash
    $ git clone https://github.umn.edu/ivlab-cs/VirtualLab.git
    $ cd VirtualLab
    ```
    
## Get ready with Docker

1. Set up the system and build the environment

    ```bash
    $ bin/setup.sh
    $ bin/build-dev.sh
    ```
If the commands return "-bash: bin/build-env.sh: No such file or directory", make sure that you add docker command without using sudo. This can be done by following step 2 in https://www.digitalocean.com/community/tutorials/how-to-install-and-use-docker-on-ubuntu-18-04.
Remember that after you follow step 2, close your terminal, open a new one and try again. 

2. Make docker environment

    ```bash
    #Usage bin/run-env.sh <port - optional(default 8081)>
    $ bin/run-env.sh
    $ make
    ```
    
This will lead to to a docker cell. 

3. Run the docker image

    ```bash
    $ build/bin/CellModel &
    $ build/bin/VLExplorer 8081 src/VLExplorer/web
    ```
4. Open up a web browser and browse to http://127.0.0.1:8081/

5. To exit docker, simply type
    ```bash
    $ exit
    ```

