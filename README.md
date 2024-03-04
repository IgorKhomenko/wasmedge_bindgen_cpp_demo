# Overview

The project's goal is to mimic the `wasmedge-bindgen` behaviour at C++ side for the Host app.

# How to 

Build rust wasm user module:

```
cd module && cargo build --target wasm32-wasi --release && cd ..
```

Run host app:

```
mkdir build && cd build
cmake ..
cd ..

cd build && make && ./app-externref && cd ..
```

How to debug:

```
lldb --file app-externref
env DYLD_LIBRARY_PATH=/Users/dev/.wasmedge/lib
r
```
https://stackoverflow.com/a/63943980