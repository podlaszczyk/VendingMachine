FROM ubuntu:22.04 as BuildServerQtStage

ENV DEBIAN_FRONTEND=noninteractive \
    DEBCONF_NONINTERACTIVE_SEEN=true

ARG CMAKE_VERSION=3.27.9
ARG LINUX_DEPLOY_TAG=1-alpha-20231026-1
ARG LINUX_DEPLOY_PLUGIN_QT_TAG=continuous
ARG QT_VERSION=6.5.3

RUN apt update && apt upgrade -y
RUN apt update && apt install -y  \
    build-essential  \
    perl  \
    git  \
    wget  \
    libssl-dev  \
    ninja-build \
    libfontconfig1-dev  \
    libfreetype6-dev  \
    libx11-dev  \
    libx11-xcb-dev  \
    libxext-dev  \
    libxfixes-dev \
    libxi-dev \
    libxrender-dev \
    libxcb1-dev  \
    libxcb-cursor-dev \
    libxcb-glx0-dev \
    libxcb-keysyms1-dev \
    libxcb-image0-dev \
    libxcb-shm0-dev  \
    libxcb-icccm4-dev  \
    libxcb-sync-dev  \
    libxcb-xfixes0-dev  \
    libxcb-shape0-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-util-dev \
    libxcb-xinerama0-dev \
    libxcb-xkb-dev  \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libxkbcommon0 \
    libgl1-mesa-glx  \
    mesa-common-dev \
    libglx-dev  \
    libgl1-mesa-dev \
    libatspi2.0-dev \
    libsqlite3-dev \
    sqlite3

RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz && \
    tar -zxvf cmake-${CMAKE_VERSION}.tar.gz && \
    cd cmake-${CMAKE_VERSION} && \
    ./bootstrap --prefix=/usr/local/ && \
    make -j 16 && \
    make install && \
    rm -rf cmake-${CMAKE_VERSION}.tar.gz cmake-${CMAKE_VERSION}

RUN git clone --branch=v${QT_VERSION} git://code.qt.io/qt/qt5.git qt-sources
RUN cd qt-sources && ./init-repository --module-subset=qtbase,qthttpserver,qtserialport,qtdeclarative
RUN mkdir -p qt-sources/build \
    && cd qt-sources/build \
    && ../configure -opensource -confirm-license -widgets -submodules qthttpserver,qtserialport,qtdeclarative -gui --xcb -nomake examples -nomake tests -prefix /usr/local/Qt/${QT_VERSION}/gcc_64 \
    && cmake --build . \
    && cmake --install . \
    && cd ../.. \
    && rm -rf qt-sources

RUN apt install -y libboost-filesystem-dev libboost-regex-dev cimg-dev patchelf nlohmann-json3-dev

RUN git clone https://github.com/linuxdeploy/linuxdeploy.git --branch ${LINUX_DEPLOY_TAG} --recursive /tmp/linuxdeploy \
    && mkdir -p /tmp/linuxdeploy/build \
    && cd /tmp/linuxdeploy/build \
    && cmake .. -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release -DUSE_CCACHE=OFF -DBUILD_TESTING=OFF \
    && cmake --build . \
    && cmake --install .

RUN git clone https://github.com/linuxdeploy/linuxdeploy-plugin-qt.git --branch ${LINUX_DEPLOY_PLUGIN_QT_TAG} --recursive /tmp/linuxdeploy-plugin-qt \
    && mkdir -p /tmp/linuxdeploy-plugin-qt/build \
    && cd /tmp/linuxdeploy-plugin-qt/build \
    && cmake .. -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release -DUSE_CCACHE=OFF -DBUILD_TESTING=OFF \
    && cmake --build . \
    && cmake --install .


FROM ubuntu:22.04 as BuildServerCodeStage

ARG LLVM_TOOLS_VERSION=17

ENV DEBIAN_FRONTEND=noninteractive \
    DEBCONF_NONINTERACTIVE_SEEN=true

COPY --from=BuildServerQtStage /usr/local/ /usr/local/

RUN ls /usr/local

RUN apt update && apt upgrade -y

RUN apt update && apt install software-properties-common -y \
    && add-apt-repository ppa:deadsnakes/ppa -y  \
    && apt update && apt install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    git git-lfs ssh \
    libgl1-mesa-glx mesa-common-dev libglx-dev libgl1-mesa-dev \
    libpcre2-16-0 \
    ninja-build \
    python3.12 python3-pip \
    unzip zip \
    wget \
    gcovr lcov \
    libfontconfig1-dev libfreetype6-dev libx11-dev libx11-xcb-dev libxext-dev libxfixes-dev libxi-dev \
    libxrender-dev libxcb1-dev libxcb-cursor-dev libxcb-glx0-dev libxcb-keysyms1-dev libxcb-image0-dev \
    libxcb-shm0-dev libxcb-icccm4-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-randr0-dev \
    libxcb-render-util0-dev libxcb-util-dev libxcb-xinerama0-dev libxcb-xkb-dev libxkbcommon-dev \
    libxkbcommon-x11-dev libxkbcommon0 \
    libjpeg-turbo8 patchelf \
    socat  curl\
    libsqlite3-dev \
    catch2 \
    gdb \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install conan==1.63.0 \
    && conan profile new default --detect \
    && conan profile update settings.compiler.libcxx=libstdc++11 default

RUN apt update && DEBIAN_FRONTEND=noninteractive apt-get install -y x11-apps

ARG USERNAME=myuser
ARG GROUPNAME=mygroup
ARG USERID=1000
ARG GROUPID=1000

RUN groupadd --gid $GROUPID $GROUPNAME \
    && useradd --uid $USERID --gid $GROUPID --shell /bin/bash --create-home $USERNAME \
    && usermod -aG sudo myuser \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

WORKDIR /asd
COPY . /asd

# Execute when project is ready

RUN mkdir -p /asd/build

#RUN conan install /asd --build=missing

#RUN  cd /asd/build \
#    && cmake .. -G Ninja -DCMAKE_PREFIX_PATH="/usr/local/Qt/6.5.3/gcc_64/" \
#    && cmake --build . --target all \
#    && cmake --build . --target VendingMachine

FROM ubuntu:22.04 as RunnableStage

COPY --from=BuildServerCodeStage /asd/build/VendingMachine/ /asd/VendingMachine/

RUN apt update && apt upgrade -y
RUN apt-get install -y socat libgl1 x11-apps libgl1-mesa-glx  libegl1-mesa libegl1 libopengl0