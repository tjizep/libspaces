****LIB SPACES API GUIDE****
==================

spaces namespace
***
**Initialization**
---
open()

usage

    local s = spaces.open()

**description**

Opens the root persisted object. It is not initialized the first time the 
file system is created. It will be initialized the moment the first object/node is created.


    if s.child == nil then
        s.child = { age=5, ["favorite color"]="green" }
        spaces.commit()
    end    

---
storage(name)

usage

    spaces.storage("client")

**description**

creates the data files in the _client_ directory. This directory should exist. If it does not then
the dbms will proceed in memory only mode.

---
localWrites(on)

usage
    
    spaces.localWrites(false)
    
**description**

If the parameter is __false__ any writes to local storage including journalling entries are suppressed. __true__ will start writing again.

---
serve(port)

usage

    spaces.serve(16003)

**description**

will start a server on *port* and block the calling thread. The server will service block requests only while the client 
will translate these blocks into a tree and hashtable structures.

---
observe(ip address, port)

usage
    
    spaces.observe("192.168.0.10", 15003)
    
**description**

when a server connection is established each client becomes part of the replication cluster. This will give the server 
a callback destination after the first handshake so that any changes made to blocks by other clients
will notify this client so that it may invalidate the appropriate caches locally.

---
replicate(ip address, port)

usage
    
    spaces.replicate("192.168.0.11", 16003)

**description**   

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
---
**transaction api's**
---------------------


read()

usage
    
    spaces.read()

**description**   

Starts a read only transaction. A version on the internal MVCC (Multi Version Concurency Control) stack is locked 
and all reads are issued using those blocks. if replicate(...) was configured all blocks are retrieved from the 
specified server/s. Any previous transactional state is discarded if it was **write** mode. isolation is serializable

---

write()

usage
    
    spaces.write()

**description**   

Starts a read/write transaction that will lock resources. Isolation is serializable. use commit to persist changes.

**NOTE: a write transaction is also started and any existing readstate is expelled if an assignment takes place during
a read transaction.**

i.e. 
    s.name = 'test'
    
**if there are only reads then no transactional state change takes place so a write will remain write and 
commit will have no effect.**

---

commit()

usage
    
    spaces.commit()

**description**   

Commits a read/write transaction that will persist modified resources. Isolation is serializable.

---

rollback()

usage
    
    spaces.rollback()

**description**   

Reverse changes during a read/write transaction or starts a readonly transaction on the latest version if the current one is stale. Isolation is serializable.


***
graph structure manipulation operators
--------------------------------------

let a the container _graph_ be defined as follows:

    local graph = spaces.open()

we can add a node **n1** to container _graph_ 

    graph.n1 = {}
    
we can create another node in _graph_ called **n2**

    graph.n2 = {}

adding an edge named _e1_ from **n1** to **n2**
    
    graph.n1.e1 = graph.n2
   
the node *n1* can also have properties p1 and p2

    graph.n1.p1 = 'v1'
    graph.n1.p2 = 'v2'

It follows that a graph and a node is equivalent and _n1_ 
is a subgraph of _graph_

Properties are neither nodes nor edges.

To remove the path to a subgraph **n2** from **graph**

    graph.n2 = nil
  
***
range queries on nodes
---
the edges and properties on a node can be queried in order with the range functor
assuming that graph is popuated as follows

    graph = {n1={},n2={},n3={},n4={},n5={},n6={}}

    graph('n3','n5')

returns a closure such that
    
    for k,v in graph('n3', 'n5') do
        print(k)
    end
 
produces
    
    n3
    n4
    n5
 
 It should be noted that the initial value of graph could have been 
    
    graph = {n1='v1', n2='v2', n3='v3', n4='v4'...}
    
With the same output as above.

---


