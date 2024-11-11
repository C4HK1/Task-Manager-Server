FROM archlinux

RUN pacman-db-upgrade

RUN pacman -Syyu --noconfirm

RUN pacman -Syyu --noconfirm make \ 
    cmake \
    gcc \
    git \
    nlohmann-json \
    openssl \
    linux-headers \
    gtest \
    wget

RUN wget -c 'http://sourceforge.net/projects/boost/files/boost/1.83.0/boost_1_83_0.tar.bz2/download' && \
    tar --bzip2 -xf download && \
    cd boost_1_83_0 && \
    ./bootstrap.sh && \
    ./b2 install



COPY . /app

RUN cd /app && \
    rm -rf libs && \
    mkdir libs && \
    cd libs && \
    rm -rf cpp-jwt && \
    git clone https://github.com/arun11299/cpp-jwt.git && \
    cd cpp-jwt && \
    mkdir build && \
    cd build && \
    cmake .. && \
    cmake --build . -j

RUN cd /app && \
    rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DCMAKE_C_COMPILER=/usr/bin/gcc .. && \
    cmake --build . 

WORKDIR /app/build

ENTRYPOINT [ "./RestfulApi", "0.0.0.0", "8080"]
EXPOSE 8080