
build poppler: 
```
mkdir build && cd build && cmake -DENABLE_UNSTABLE_API_ABI_HEADERS=ON ..
make && sudo make install
```

基于 poppler-0.79 开发。
