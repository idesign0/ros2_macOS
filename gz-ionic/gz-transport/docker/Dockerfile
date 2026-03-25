# Ubuntu 24.04
FROM ubuntu:noble

ENV TZ=America/Los_Angeles
ENV DEBIAN_FRONTEND=noninteractive
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Tools I find useful during development
RUN apt-get update \
 && apt-get install -y \
        cmake \
        pkg-config \
        cppcheck \
        git \
        build-essential \
        curl \
        libprotobuf-dev \
        protobuf-compiler \
        libprotoc-dev \
        libzmq3-dev \
        net-tools \
        uuid-dev \
        doxygen \
        ruby-ronn \
        libsqlite3-dev \
        python3-pybind11 \
        sudo \
        gnupg \
        lsb-release \
        wget \
        tzdata \
 && apt-get clean

# Install gazebo dependencies
RUN /bin/sh -c 'echo "deb [trusted=yes] http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list' \
 && /bin/sh -c 'wget http://packages.osrfoundation.org/gazebo.key -O - | apt-key add -' \
 && apt-get update \
 && apt-get install -y \
    libgz-cmake4-dev \
    libgz-math8-dev \
    libgz-msgs11-dev \
    libgz-utils3-cli-dev \
    libgflags-dev \
 && apt-get clean

USER ubuntu
WORKDIR /tmp
# Gazebo transport
RUN git clone https://github.com/gazebosim/gz-transport.git -b gz-transport14
RUN cd gz-transport \
 && mkdir build \
 && cd build \
 && cmake .. -DCMAKE_INSTALL_PREFIX=/tmp/gz-transport \
 && make -j4 install

# Gazebo transport examples
RUN cd gz-transport/example \
 && mkdir build \
 && cd build \
 && cmake .. -DCMAKE_INSTALL_PREFIX=/tmp/gz-transport \
 && make -j4

WORKDIR /tmp/gz-transport/example/build

# Customize your image here.
# E.g.:
# ENV PATH="/opt/sublime_text:$PATH"
