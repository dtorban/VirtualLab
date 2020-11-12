# Builder image
FROM ubuntu:18.04 as builder
RUN groupdel dialout
RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    git \
    libssl-dev \
    zlib1g-dev \
    dos2unix \
    rsync \
    libmysqlcppconn-dev \
    cmake \
    libgtest-dev

ARG DEP_DIR=/dependencies
ARG SETUP_DIR=/env

RUN mkdir -p ${SETUP_DIR}
RUN mkdir -p ${DEP_DIR}
RUN git clone https://github.com/dtorban/CppWebServer.git ${SETUP_DIR}/CppWebServer 
RUN mkdir -p ${SETUP_DIR}/CppWebServer/build
WORKDIR ${SETUP_DIR}/CppWebServer/build
RUN cmake -DCMAKE_INSTALL_PREFIX=${DEP_DIR} ..
RUN make install -j

RUN find ${DEP_DIR} -type d -exec chmod 775 {} \;
RUN find ${DEP_DIR} -type f -exec chmod 664 {} \;

# Dev image
FROM builder as dev
ARG USER_ID
ARG GROUP_ID
RUN addgroup --gid $GROUP_ID user
RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID user
RUN mkdir -p /home/user
WORKDIR /home/user/vl
USER user
