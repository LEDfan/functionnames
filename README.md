Function Names
===

Tool to check if a C++ project is using function and method names which start with a capital.

Uses the [cppast](https://github.com/foonathan/cppast) library.

## Installation

```
git submodule update --init
```

### openSUSE
Install the [llvm5-devel](https://software.opensuse.org/package/llvm5-devel) and [clang5-devel](https://software.opensuse.org/package/clang5-devel?search_term=clang5-devel) packages.

### Ubuntu 16.04

 - Add a ppa:
```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
```

 - Install packages and update alternatives:
```
sudo apt install llvm-5.0-dev libclang-5.0-dev clang-5.0
sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-5.0 100
```

### Build and install

Build like any cmake project:

```
mkdir build
cd build
cmake ..
make
sudo make install
```




