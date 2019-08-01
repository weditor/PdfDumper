
build poppler: 
```
mkdir build && cd build && cmake -DENABLE_UNSTABLE_API_ABI_HEADERS=ON ..
make && sudo make install
```
