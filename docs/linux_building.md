## Building
Example from SteamRT environment
* CMake with presets support >= 3.19
* System compiler GCC-10, but additionally GCC-12 installed for C++20 support
* Ninja generator package installed (`sudo apt install ninja-build`)

Working directory: repository root
```
cmake --preset Release -D CMAKE_C_COMPILER=gcc-12 -D CMAKE_CXX_COMPILER=g++-12
cmake --build --preset Release -t py12-3-lang-module -j
```

#### Without presets support, with system compiler
```
cmake -G Ninja -B build/Linux/Debug -D CMAKE_BUILD_TYPE=Debug
cmake --build build/Linux/Debug -t py12-3-lang-module -j
```

## Python building
Repository python binaries was built with:  
Python 3.12.1 from https://github.com/python/cpython/tree/v3.12.1  
For example, path to repo `/home/myuser/cpython`, replace to yours  
Environment: SteamRT  
Working directory: cpython repository root  
```
mkdir -p build/release_install
mkdir -p build/release
cd build/release
../../configure --enable-shared --enable-optimizations --prefix=/home/myuser/cpython/build/release_install
make CC=gcc-12 CXX=g++-12 -j
make altinstall
```
#### Debug
```
mkdir -p build/debug_install
mkdir -p build/debug
cd build/debug
../../configure --enable-shared --with-pydebug --prefix=/home/myuser/cpython/build/debug_install
make CC=gcc-12 CXX=g++-12 -j
make altinstall
```
