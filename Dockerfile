FROM ubuntu:16.04
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && \
    apt-get -y install gcc mono-mcs && \
    rm -rf /var/lib/apt/lists/*
ADD server.cpp ./
RUN g++ -o server  -g -Wall -pthread -std=c++11 -O2 server.cpp
RUN mkdir ./save
VOLUME ./save
EXPOSE 2017
CMD ["./server", "2017", "save"]