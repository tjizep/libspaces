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
    static std::string to_data(const nst::version_type& version){
        std::string version_data;
        version_data.resize(16);
        version.copyTo(&version_data[0]);
        return version_data;
    }
    static nst::version_type to_version(const std::string& data){
        nst::version_type version;
        version.copyFrom(data.data());
        return version;
    }
    static void assign(nst::version_type &version, const std::string& data){
        version.copyFrom(data.data());
    }
    static void assign(std::string& version_data, const nst::version_type &version){
        version_data.resize(16);
        version.copyTo(&version_data[0]);
    }
    static const bool debug_print = true;
    static const bool debug_print_client = false;
    typedef std::shared_ptr<stored::abstracted_storage> storage_ptr;
    block_replication_server::block_replication_server(nst::u16 port) : port(port){

    }
    /// TODO: nb must verify timing between server an client, i.e. a call to begin must
    /// return a version younger than the initial version but older than a new version
    /// on the client side. An inverse of this can also be performed on the server side.
    /// the changes to state must be orderable between server and client
    ///
    void block_replication_server::run(){
        storage_ptr storage ;
        rpc::server srv(this->port); // listen on TCP port
        srv.bind("open", [&storage](bool is_new,const std::string& name) {
            dbg_print("open(%s)", name.c_str());
            if(storage==nullptr) storage = std::make_shared<stored::abstracted_storage>(name);

        });
        srv.bind("is_open", [&storage]() {
            return (storage!=nullptr) ;

        });

        srv.bind("get", [&storage](std::string& version, nst::buffer_type& data, nst::u64 address) {
            dbg_print("get(%lld)", (nst::fi64)address);
            if(storage==nullptr) return false;
            if(storage->is_transacted()) return false;
            nst::u64 which = address;
            data = storage->allocate(which,stx::storage::read);

            assign(version,storage->get_allocated_version());
            storage->complete();
        });
        srv.bind("is_latest", [&storage](const std::string& version, nst::buffer_type& data, nst::u64 address) {
            dbg_print("get(%lld)", (nst::fi64)address);
            if(storage==nullptr) return false;
            if(storage->is_transacted()) return false;
            nst::u64 which = address;

            return storage->is_latest(which,to_version(version));


        });
        srv.bind("store", [&storage](nst::u64 address, const nst::buffer_type& data) {
            dbg_print("store(%lld,%lld)", (nst::fi64)address, (nst::fi64)data.size() );
            if(storage==nullptr) return false;
            if(storage->is_transacted()) return false;
            nst::u64 which = address;
            nst::buffer_type& writeable = storage->allocate(which,stx::storage::create);
            writeable = data;
            storage->complete();
        });
        srv.bind("begin", [&storage](bool write) {
            dbg_print("begin(%lld)", (nst::fi64)write);
            if(storage==nullptr) {
                err_print("storage not available to start transaction");
                return ;
            }
            storage->begin(write);

        });
        srv.bind("beginVersion", [&storage](bool write,const std::string& version) {
            dbg_print("begin(%lld)", (nst::fi64)write);

            if(storage==nullptr) {
                err_print("storage not available to start transaction");
                return ;
            }
            storage->begin(write,to_version(version));

        });
        srv.bind("commit", [&storage]() {
            dbg_print("commit()");
            if(storage==nullptr) {
                err_print("storage not available to commit transaction");
                return false;
            }
            try{
                storage->commit();
                return true;
            }catch(std::exception& e){
                err_print("error committing transaction %s, client notified",e.what());
            }
            return false;


        });
        srv.bind("rollback", [&storage]() {
            dbg_print("rollback()");
            if(storage==nullptr) {
                err_print("storage not available to rollback transaction");
                return ;
            }
            storage->rollback();

        });

        srv.bind("max_block_address", [&storage]()->nst::u64 {
            dbg_print("max_block_address()");
            if(storage==nullptr) {
                err_print("storage not available to get max address");
                return 0;
            }
            return storage->get_max_block_address();

        });
        srv.bind("contains", [&storage](nst::u64 address) {
            dbg_print("contains(%lld)", (nst::fi64)address);
            if(storage==nullptr) {
                err_print("storage not available to verify address");
                return false;
            }
            return storage->contains(address);

        });
        srv.bind("close", [&storage]() {
            dbg_print("close()");
            storage=nullptr;
        });
        std::cout << "spaces server listening on port " << this->port << std::endl;
        srv.run();
    }
    block_replication_server::~block_replication_server(){

    }
/// client
    block_replication_client::block_replication_client(const std::string& address, nst::u16 port)
    :   address(address)
    ,   port(port){



    }
    block_replication_client::block_replication_client() : port(spaces::DEFAULT_PORT){

    }
    void block_replication_client::set_address(const std::string& address){
        this->address = address;
        this->remote = nullptr;

    }
    const char * block_replication_client::get_address() const {
        return  address.c_str();
    }
    nst::fi64 block_replication_client::get_port() const {
        return port;
    }
    bool block_replication_client::is_open() const {
        dbg_print("is_open()");
        if(remote==nullptr) return false;
        try{
            return this->remote->call("is_open").as<bool>(); /// TODO: this adds latency but its safe
        }catch(std::exception& e){
            dbg_print("error check is_open: %s on %s:%lld",e.what(),get_address(),get_port());
        }
        return false;

    }
    void block_replication_client::close() {
        dbg_print("close()");
        if(this->remote != nullptr){
            try{
                this->remote->call("close");
            }catch(std::exception& e){
                dbg_print("error closing: %s on %s:%lld",e.what(),get_address(),get_port());
            }
            try{
                this->remote = nullptr; /// destructor should not throw ???
            }catch(std::exception& e){
                dbg_print("error destroying: %s on %s:%lld",e.what(),get_address(),get_port());
                /// TODO: probably exit(e) at this point
            }
        }
    }
    void block_replication_client::begin(bool is_write){

        dbg_print("begin(%lld)",(nst::fi64)is_write);
        if(this->remote == nullptr) return;
        this->remote->call("begin",is_write);
    }
    void block_replication_client::begin(bool is_write,const nst::version_type& version){
        dbg_print("beginVersion(%lld,%s)",(nst::fi64)is_write, nst::tostring(version));
        if(this->remote == nullptr) return;
        this->remote->call("beginVersion",is_write,to_data(version));
    }
    bool block_replication_client::commit(){
        dbg_print("commit()");
        if(this->remote == nullptr) return false;
        try{
            return this->remote->call("commit").as<bool>();
        }catch(std::exception& e){
            dbg_print("error commiting: %s on %s:%lld",e.what(),get_address(),get_port());
        }
        return false;
    }

    void block_replication_client::rollback(){
        dbg_print("rollback()");
        if(this->remote == nullptr) return;
        this->remote->call("rollback");
    }
    void block_replication_client::open(bool is_new,const std::string& name){
        dbg_print("open(%lld,%s)",(nst::fi64)is_new, name.c_str());
        if(this->address.empty()) return; /// allows a local instance to be its own server
        if(this->remote == nullptr)
            this->remote = std::make_shared<rpc::client>(this->address, port);
        this->remote->call("open", is_new, name);
    }
    void block_replication_client::store(nst::u64 address, const nst::buffer_type& data){
        dbg_print("store(%lld,[%lld])",(nst::fi64)address, (nst::fi64)data.size());
        if(this->remote == nullptr) return;
        this->remote->call("store", address, data);
    }
    bool block_replication_client::get(nst::version_type& version, nst::buffer_type& data, nst::u64 address){
        dbg_print("get(%s, %lld, [%lld])", nst::tostring(version),(nst::fi64)address, (nst::fi64)data.size());
        if(this->remote == nullptr)
            return false;

        return this->remote->call("get", to_data(version), data, address).as<bool>();
    }
    bool block_replication_client::is_latest(const nst::version_type& version, nst::u64 address){
        dbg_print("is_latest(%s, %lld)",version.toString().c_str() ,(nst::fi64)address);
        std::string version_data;
        version_data.resize(16);
        version.copyTo(&version_data[0]);
        return this->remote->call("is_latest", version_data, address).as<bool>();
    }
    nst::u64 block_replication_client::max_block_address() const {
        dbg_print("max_block_address()");
        if(this->remote == nullptr) return 0;
        return this->remote->call("max_block_address").as<nst::u64>();
    }
    bool block_replication_client::contains(nst::u64 address){
        dbg_print("contains(%lld)",(nst::fi64)address);
        if(this->remote == nullptr) return false;
        return this->remote->call("contains").as<nst::u64>();
    }
    block_replication_client::~block_replication_client(){
        this->close();
    }
    repl_client_ptr create_client(const std::string& address, nst::u16 port){
        return std::make_shared<block_replication_client>(address,port);
    }
}