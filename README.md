
```
mkdir build && cd build
cmake ..
cd ..

cd build && make && ./app-externref
```

========

```
cd module && cargo build --target wasm32-wasi --release && cd ..

gcc main.c -lwasmedge && ./a.out
```
