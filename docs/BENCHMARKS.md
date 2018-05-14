****LIB SPACES****
--

_BENCHMARK(eting) UNDER CONSTRUCTION_

Here are some initial values for a benchmark run.

Data Set
    
    1,000,000 keys of 8 bytes each randomly generated with values 8 bytes each

Platform

    linux mint 18.3 64-bit
    GCC 4.6
    luajit 2.1-torch
    Intel core i7 8700 @ 3.6 GHz x 6
    16 Gb RAM
    ADATA SSD 512 Gb
    

Random write rate

    1,204,473 keys/s

Hot reads

    2,095,285 keys/s	1

    3,229,525 keys/s	2

Cold reads

    2,053,708 keys/s	1

    3,190,729 keys/s	2

Please take these with a grain of salt and do not assume the performance will hold under
all circumstances. For reference REDIS performance on the same system and compilers 
    
    redis-benchmark -n 1000000 -r 1000000 -t set,get -P 256 -q
    SET: 1360544.25 requests per second
    GET: 1663893.50 requests per second
