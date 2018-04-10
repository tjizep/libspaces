//
// Created by pirow on 2018/04/09.
//
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <rpc/server.h>
#include <storage/spaces/dbms.h>

#include "replication.h"

namespace spaces{
    struct storage_message{
        nst::stream_address address;
        nst::buffer_type buffer;
        MSGPACK_DEFINE_ARRAY(address,buffer);
    };
    dbms db(STORAGE_NAME);
    void store_data(const storage_message& msg){

    }
    replication_server::replication_server(nst::u16 port) : port(port){

    }
    void replication_server::run(){
        rpc::server srv(this->port); // listen on TCP port
        srv.bind("store", [](storage_message const& c) {

            return "ok";

        });

        srv.bind("get", [](storage_message& c) {

            return "ok";

        });

        srv.bind("begin", [](bool read) {

            return "ok";

        });

        srv.bind("commit", []() {

            return "ok";

        });
        srv.bind("rollback", []() {

            return "ok";

        });
    }
    replication_server::~replication_server(){

    }
}