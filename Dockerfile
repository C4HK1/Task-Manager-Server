FROM alpine

RUN apk update

RUN apk add boost-dev \
    cmake \
    linux-headers \
    make \
    g++ \
    nlohmann-json \
    gtest-dev \
    libressl-dev \
    git

COPY . /app

RUN cd /app && \
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
    make 

WORKDIR /app/build

ENTRYPOINT [ "./RestfulApi", "0.0.0.0", "8080"]
EXPOSE 8080