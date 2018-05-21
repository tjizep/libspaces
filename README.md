****LIB SPACES**** v0.3.2
--

    So, What is it ?

Lib(eration) spaces is a library and server for transactional graph and **ordered** key value storage/persistence. 

    And...
It is flexible in that it can operate as a pure embedded database, memory only, distributed with master master 
replication and the traditional client server mode.
  

    Accessing it

It is designed to be accessed in a non schema oriented way and lends itself to dynamic languages like javascript, 
lua and groovy (*Only* the ones I like **{he,he}**)

Some would describe the interface as natural and permits the database as a first class citizen to your code as opposed 
to **_apart from_** your code.

    Any examples or tutorials ?

****Yes!**** please follow the examples and tutorial links 
1. [tutorial](docs/TUTORIAL.md) 
2. [examples](docs/EXAMPLES.md)
3. [api reference](docs/API.md)
 
You can also check out the performances at
[benchmarks](docs/BENCHMARKS.md) 

    Ok thats great how can I install it
Install via luarocks (npm will be available soon) [luarocks](https://luarocks.org/)
    
    > luarocks install --server=http://luarocks.org/manifests/tjizep spaces
  
 The only further requirements are for boost min version =? 1.58 to be installed 
 and cmake i.e.
 
    > apt-get install libboost-all-dev 

