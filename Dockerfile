FROM mull-dependencies:latest

COPY . mull

# Build Mull
RUN set -ex \
    && cd mull \
    && mkdir build.dir \
    && cd build.dir \
    && ls -l /usr/share/llvm-6.0/cmake \
    && CC=clang CXX=clang++ Clang_DIR=/usr/share/llvm-6.0/cmake cmake -DPATH_TO_LLVM=/usr/lib/llvm-6.0 .. \
    && make mull-cxx \
    && make mull-tests
