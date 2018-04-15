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
    typedef std::shared_ptr<stored::abstracted_storage> storage_ptr;
    block_replication_server::block_replication_server(nst::u16 port) : port(port){

    }
    void block_replication_server::run(){
        storage_ptr storage ;
        rpc::server srv(this->port); // listen on TCP port
        srv.bind("open", [&storage](bool is_new,const std::string& name) {
            std::cout << "open(" << name << ")" << std::endl;

            if(storage==nullptr) storage = std::make_shared<stored::abstracted_storage>(name);

        });
        srv.bind("is_open", [&storage]() {
            return (storage!=nullptr) ;

        });

        srv.bind("get", [&storage](std::string& version, nst::buffer_type& data, nst::u64 address) {
            std::cout << "get(" << address << ")" << std::endl;
            if(storage==nullptr) return false;
            if(storage->is_transacted()) return false;
            nst::u64 which = address;
            data = storage->allocate(which,stx::storage::read);
            version.resize(16);
            storage->get_allocated_version().copyTo(&version[0]);
            storage->complete();
        });
        srv.bind("store", [&storage](nst::u64 address, const nst::buffer_type& data) {
            std::cout << "store(" << address << ")" << std::endl;
            if(storage==nullptr) return false;
            if(storage->is_transacted()) return false;
            nst::u64 which = address;
            nst::buffer_type& writeable = storage->allocate(which,stx::storage::create);
            writeable = data;
            storage->complete();
        });
        srv.bind("begin", [&storage](bool read) {
            std::cout << "begin(" << read << ")" << std::endl;
            if(storage==nullptr) {
                err_print("storage not available to start transaction");
                return ;
            }
            storage->begin(!read);

        });

        srv.bind("commit", [&storage]() {
            std::cout << "commit()" << std::endl;
            if(storage==nullptr) {
                err_print("storage not available to commit transaction");
                return ;
            }
            storage->commit();

        });
        srv.bind("rollback", [&storage]() {
            std::cout << "rollback()" << std::endl;
            if(storage==nullptr) {
                err_print("storage not available to rollback transaction");
                return ;
            }
            storage->rollback();

        });

        srv.bind("max_block_address", [&storage]()->nst::u64 {
            std::cout << "max_block_address()" << std::endl;
            if(storage==nullptr) {
                err_print("storage not available to get max address");
                return 0;
            }
            return storage->get_max_block_address();

        });
        srv.bind("contains", [&storage](nst::u64 address) {
            std::cout << "contains(" << address << ")" << std::endl;
            if(storage==nullptr) {
                err_print("storage not available to verify address");
                return false;
            }
            return storage->contains(address);

        });
    }
    block_replication_server::~block_replication_server(){

    }
/// client
    block_replication_client::block_replication_client(){

    }
    void block_replication_client::set_address(const std::string& address){
        this->address = address;
        this->remote = nullptr;

    }
    bool block_replication_client::is_open() const {
        if(remote==nullptr) return false;
        return this->remote->call("is_open").as<bool>();
    }
    void block_replication_client::close() {
        if(this->remote != nullptr){
            this->remote->call("close");
            this->remote = nullptr;
        }
    }
    void block_replication_client::begin(bool is_read){
        if(this->remote == nullptr) return;
        this->remote->call("begin",is_read);
    }
    void block_replication_client::commit(){
        if(this->remote == nullptr) return;
        this->remote->call("commit");
    }

    void block_replication_client::rollback(){
        if(this->remote == nullptr) return;
        this->remote->call("rollback");
    }
    void block_replication_client::open(bool is_new,const std::string& name){
        if(this->address.empty()) return; /// allows a local instance to be its own server
        if(this->remote == nullptr)
            this->remote = std::make_shared<rpc::client>(this->address, spaces::DEFAULT_PORT);
        this->remote->call("open", is_new, name);
    }
    void block_replication_client::store(nst::u64 address, const nst::buffer_type& data){
        if(this->remote == nullptr) return;
        this->remote->call("store", address, data);
    }
    bool block_replication_client::get(nst::version_type& version, nst::buffer_type& data, nst::u64 address){
        if(this->remote == nullptr)
            return false;
        std::string version_data;
        version_data.resize(16);
        version.copyTo(&version_data[0]);
        return this->remote->call("store", version_data, data, address).as<bool>();
    }
    nst::u64 block_replication_client::max_block_address() const {
        if(this->remote == nullptr) return 0;
        return this->remote->call("max_block_address").as<nst::u64>();
    }
    bool block_replication_client::contains(nst::u64 address){
        if(this->remote == nullptr) return false;
        return this->remote->call("contains").as<nst::u64>();
    }
    block_replication_client::~block_replication_client(){
        this->close();
    }
}