**EXAMPLES**
--

In the examples directory you can find the following lua code:

**CITIES**

Contains an example of using [jason path](../src/examples/cities/cities.lua) queries to return subsets of data in a graph like structure

**CLIENT/SERVER**

Running a [server](../src/examples/server/server.lua) and its storage -less [client](../src/examples/server/client.lua)

**GRAPH QUERIES ON SHOE STORES**

See the [jason path](../src/examples/cities/cities.lua) example for querying a simple graph of a bookstore

**NAVIGABLE SMALL WORLDS SEMANTIC SEARCH**

A graph with the small world property is created to index multi dimensional data (glove vectors) and use it
to perform a semantic and levenshtein [search](../src/examples/search/search.lua).

**BENCHMARK**

A simple [benchmark](../src/examples/benchmark/benchmark.lua) showing random reads and writes using spaces as a key/value store emulation. Its still quite a bit faster than most real key value stores.
