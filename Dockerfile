FROM gcc:latest

WORKDIR /mass
COPY source/ ./source/
COPY ubuntu/ ./ubuntu/

# prepare libssh2
WORKDIR /mass/ubuntu/libssh2-1.9.0/
RUN ./configure --prefix=/mass/ubuntu/ssh2 --enable-debug
RUN make
RUN make install

# prepare mass
WORKDIR /mass/ubuntu
RUN ls
RUN make

# setup environment variable
ENV LD_LIBRARY_PATH=/mass/ubuntu:/mass/ubuntu/ssh2/lib
