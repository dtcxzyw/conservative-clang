# Conservative-Clang

Conservative-Clang: An early LLVM pass which removes most of UB-implying information.

If you are fighting with C/C++ undefined behavior (or clang miscompilation bugs), please try this tool :)

Before using this pass, please try with the following compilation flags in [clang-flags.txt](./clang-flags.txt):

```
-fwrapv -fno-check-new -fno-delete-null-pointer-checks -fno-strict-aliasing -fno-strict-enums -fno-strict-float-cast-overflow -fno-strict-return -fno-strict-vtable-pointers -fno-struct-path-tbaa -fno-pointer-tbaa -fno-assume-nothrow-exception-dtor -fno-assume-sane-operator-new -fno-assume-unique-vtables -fno-assumptions -fno-finite-loops -fno-common -mno-global-merge -fno-merge-all-constants
```

If these "magical formula" does not work, please try this pass.

This work is inspired by Daniel J. Bernstein's [blog](https://blog.cr.yp.to/20240803-clang.html). I hope this work can unblock your project at the prototype stage.

## Getting Started

### Installation

```bash
sudo apt install llvm-20-dev # Depends on your clang version
git clone https://github.com/dtcxzyw/conservative-clang.git
cd conservative-clang
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
```

### Usage

```bash
export CFLAGS=-fpass-plugin=<path to conservative-clang>/build/cclang.so
export CXXFLAGS=-fpass-plugin=<path to conservative-clang>/build/cclang.so
# Configure & build your project
```

If you are using CMake, please pass `-DCMAKE_C_FLAGS=-fpass-plugin=<path to conservative-clang>/build/cclang.so -DCMAKE_CXX_FLAGS=-fpass-plugin=<path to conservative-clang>/build/cclang.so` to CMake.

## Performance

I did some performance evaluation on llvm test-suite. This pass can result in ~10% performance degradation on average.

## License

This project is licensed under the Apache License 2.0. Please see the [LICENSE](./LICENSE) for details.
