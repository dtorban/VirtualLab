FROM  ubuntu:18.04 as dev

RUN groupdel dialout

RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    libssl-dev \
    zlib1g-dev \
    dos2unix \
    rsync \
    libmysqlcppconn-dev

ARG USER_ID
ARG GROUP_ID

RUN addgroup --gid $GROUP_ID user
RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID user

RUN mkdir -p /home/user
WORKDIR /home/user/vl

USER user
