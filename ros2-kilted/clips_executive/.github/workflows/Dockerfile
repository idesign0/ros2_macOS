ARG ROS_DISTRO=jazzy
FROM docker.io/library/ros:${ROS_DISTRO}

RUN apt-get update && apt-get install -y \
    python3-pip \
    python3-vcstool \
    python3-rosdep \
    git \
    && rm -rf /var/lib/apt/lists/*

ENV PIP_BREAK_SYSTEM_PACKAGES=1

RUN if [ "$ROS_DISTRO" = "humble" ]; then \
        pip install unified-planning; \
    else \
        pip install unified-planning --break-system-packages; \
    fi

WORKDIR /ros_ws

COPY . src/workdir

RUN find . -name "*.repos" -exec vcs import --recursive --input {} src \;

RUN rosdep update && \
    . /opt/ros/${ROS_DISTRO}/setup.sh && \
    rosdep install --from-paths src --ignore-src -r -y
