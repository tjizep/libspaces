****LIB SPACES****
-----------------

****API GUIDE****

spaces
--
**Initialization**

open()

usage

    local s = spaces.open()

Opens the root persisted object. It is not initialized the first time the 
file system is created. It will be initialized the moment the first child object is created.


    if s.child == nil then
        s.child = { age=5, ["favorite color"]="green" }
        spaces.commit()
    end    


storage(name)

usage

    spaces.storage("client")

creates the data files in the _client_ directory. This directory should exist. If it does not then
the dbms will proceed in memory only mode.

serve(port)

usage

    spaces.serve(16003)

will start a server on *port* and block the calling thread. The server will service block requests only while the client 
will translate these blocks into

observe(ip address, port)

usage
    
    spaces.observe("192.168.0.10", 15003)
    

when a server connection is established each client becomes part of the replication cluster. This will give the server 
a callback destination after the first handshake so that any changes made to blocks by other clients
will notify this client so that it may invalidate the appropriate caches locally.

replicate(ip address, port)

usage
    
    spaces.replicate("192.168.0.11", 16003)
   
Connects to a server and send all changed and or new blocks to the server listening at that
ip port combination.

A complete flow for a replication client would be

    spaces.storage("test")
    spaces.observe("192.168.0.10",15003)
    spaces.replicate("192.168.0.11",16003)
    spaces.localWrites(false)
    
Considering that the client is running on ip 192.168.0.10 and the server on 192.168.0.11
spaces.localWrites(false) simply switches off all local writes and the client will use the
server as its only means of persistence. Omitting spaces.localWrites(...) will turn this 
client into a replicant of the destination server. 

Starting an embedded server would simply be
    
    spaces.storage("test")
    local s = spaces.open()
    if s.yourname == nil then
        initialize ...
        spaces.commit()
    end
    
 