****LIB SPACES****
--

    So, What is it ?

Lib(eration) spaces is a library and server for transactional graph and key value storage/persistence. 

    And...
It is flexible in that it can operate as a pure embedded database, memory only, distributed with master master 
replication and the traditional client server mode.
  

    Accessing it

It is designed to be accessed in a non schema oriented way and lends itself to dynamic languages like javascript, 
lua and groovy (*Only* the ones I like **{he,he}**)

    Any examples or tutorials ?

****Yes!**** please follow the examples and tutorial links 
1. [tutorial](docs/TUTORIAL.md) 
2. [examples](docs/EXAMPLES.md)
 
You can also check out the performances at
[benchmarks](BENCHMARKS.md) 

    Ok thats great how can I install it
Install via luarocks (npm will be available soon)
    
    > luarocks install libspaces  

you should now be able to access it in your lua scripts 

    Theres something I want to change, But does it build?


Indeed, the following commands will *probably* work for you

Start in your home or other directory with read,write and create access privileges
    
    > wget ..libspaces...
    > unzip ..
    > cd libspaces
    > mkdir build
    > cd build
    > cmake .. -DLUA_PATH=**path to lua
    
 NOTE: the lua path should contain include and lib (sic) directories
 
 The resulting shared object 'libspaces.so' should then be available 
 for your consumption
