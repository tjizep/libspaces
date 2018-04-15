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

    void store_data(const storage_message& msg){

    }
    block_replication_server::block_replication_server(nst::u16 port) : port(port){

    }
    void block_replication_server::run(){
        rpc::server srv(this->port); // listen on TCP port
        srv.bind("is_open", []() {

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
    block_replication_server::~block_replication_server(){

    }

    block_replication_client::block_replication_client(){

    }
    bool block_replication_client::is_open() const {
        return false;
    }
    void block_replication_client::begin(bool is_read){

    }
    void block_replication_client::commit(){

    }
    void block_replication_client::rollback(){

    }
    void block_replication_client::open(bool is_new,const std::string& name){

    }
    void block_replication_client::store(nst::u64 address, const nst::buffer_type& data){

    }
    bool block_replication_client::get(nst::version_type& version, nst::buffer_type& data, nst::u64 address){

    }
    nst::u64 max_block_address(){
        return  0;
    }
    block_replication_client::~block_replication_client(){

    }
}