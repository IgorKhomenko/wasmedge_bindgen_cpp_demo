Build rust wasm:

```
cd module && cargo build --target wasm32-wasi --release && cd ..
```

Run host app:

```
mkdir build && cd build
cmake ..
cd ..

cd build && make && ./app-externref
```