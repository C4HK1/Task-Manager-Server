FROM alpine

ARG BOOST_VERSION=1.84.0
ARG CMAKE_VERSION=3.30.1
ARG NUM_JOBS=8

ENV DEBIAN_FRONTEND=noninteractive

# System locale
# Important for UTF-8
ENV LC_ALL=en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US.UTF-8

# Install CMake
RUN apk add boost-dev \
    cmake \
    make \
    g++ \
    nlohmann-json \
    gtest-dev \
    libressl-dev \
    git 

COPY . /app

RUN cd app && \
    git clone https://github.com/arun11299/cpp-jwt.git && \
    cd cpp-jwt && \
    mkdir build && \
    cd build && \
    cmake .. && \
    cmake --build . -j

RUN cd app && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DCMAKE_C_COMPILER=/usr/bin/gcc .. && \
    make 

ENTRYPOINT [ "/app/build/RestfulApi", "0.0.0.0", "8080"]
EXPOSE 8080