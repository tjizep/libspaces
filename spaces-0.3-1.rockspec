package = "spaces"
version = "0.3-1"
source = {
   url = "https://github.com/tjizep/libspaces",
   tag = "v0.3"
}
description = {
   summary = "Lib(eration) spaces is a library and server for transactional graph and key value storage/persistence. ",
   detailed = "Lib(eration) spaces is a library and server for transactional graph and key value storage/persistence. ",
   homepage = "https://github.com/tjizep/libspaces",
   license = "LGPL 2.1
}
dependencies = {}
build = {
   type = "cmake",
   variables={
     LUA_INCLUDE="$(LUA_INCDIR)",
     LUA_LIBRARIES="$(LUA_LIBDIR)",
     LUA_INSTALL="$(LIBDIR)"
   }
   modules = {
      ["spaces"] = "libspaces.so"
   },
   copy_directories = {
      "examples", "docs"
   }
}
