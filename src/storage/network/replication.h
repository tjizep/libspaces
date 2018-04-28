//
// Created by pirow on 2018/04/09.
//

#ifndef SPACES_REPLICATION_H
#define SPACES_REPLICATION_H
#include <storage/transactions/abstracted_storage.h>
#include <rpc/client.h>
#include <rpc/server.h>
namespace nst = stx::storage;
namespace spaces{
    static const nst::u16 DEFAULT_PORT = 16003;
    class block_replication_server{
    private:
        nst::u16 port;
    public:
        block_replication_server(nst::u16 port);
        void run();
        ~block_replication_server();
    };
    class block_replication_client{
    private:
        std::string address;
        nst::u16 port;
        typedef std::shared_ptr<rpc::client> client_ptr;
        client_ptr remote;
    public:
        block_replication_client(const std::string& address, nst::u16 port);
        block_replication_client();
        void set_address(const std::string& address);
        const char * get_address() const ;
        nst::fi64 get_port() const ;
        bool is_open() const ;
        bool begin(bool is_write);
        bool begin(bool is_write,const nst::version_type& version,const nst::version_type& last_version);

        bool commit();
        void rollback();
        void open(bool is_new,const std::string& name);
        bool store(nst::u64 address, const nst::buffer_type& data);
        std::tuple<bool, nst::version_type, nst::buffer_type> get(nst::u64 address);
        bool is_latest(const nst::version_type& version, nst::u64 address);
        bool contains(nst::u64 address);
        void close();
        nst::u64 max_block_address() const;
        ~block_replication_client();
    };

    typedef std::shared_ptr<spaces::block_replication_client> repl_client_ptr;

    typedef std::vector<repl_client_ptr> replication_clients;
    extern repl_client_ptr create_client(const std::string& address, nst::u16 port);

    class replication_control{
    private:
        std::string name;
        replication_clients replicated;
        std::map<nst::version_type, nst::u32> version_count;
        std::map<nst::version_type, nst::buffer_type> version_data;

    public:
        void add_replicant(const std::string &address, nst::u16 port){
            replicated.push_back(spaces::create_client(address,port));
        }
        const std::string& get_name() const {
            return this->name;
        }
        replication_control(const std::string& name) : name(name){

        }
        bool check_open_replicants(const replication_clients& replicated) const {
            nst::i32 opened = 0;
            for(auto r = replicated.begin(); r != replicated.end(); ++r){
                try{
                    if(!(*r)->is_open()){
                        (*r)->open(false,this->get_name());
                    }
                    ++opened;
                }catch(std::exception& e){
                    dbg_print("could not open a replicant %s: %s:%lld",e.what(),(*r)->get_address(), (*r)->get_port());
                }

            }
            if(opened < (replicated.size() / 2)){
                wrn_print("not enough replicants could be opened %lld of %lld", (nst::fi64) opened,(nst::fi64) replicated.size());
            }
            return opened >= (replicated.size() / 2);
        }
        /// start as many replicants as possible
        /// return false if too many failed
        bool begin_replicants(bool writer, const nst::version_type& version, const nst::version_type& last_version){
            if(replicated.empty()) return true;
            if(!check_open_replicants(replicated))
                return false;
            dbg_print("starting %lld replicants ", (nst::fi64)replicated.size());
            spaces::replication_clients ok;
            for(auto r = replicated.begin(); r != replicated.end(); ++r){
                try{
                    (*r)->begin(writer,version,last_version);
                    ok.push_back((*r));
                }catch(std::exception& e){
                    wrn_print("error starting transaction [%s] on %s:%lld",e.what(),(*r)->get_address(), (*r)->get_port());
                    (*r)->close();
                }

            }
            if(ok.size() < replicated.size() / 2){ /// at least half must succeed (democratic)
                wrn_print("too few replicants could be started %lld of %lld",(nst::fi64)ok.size() ,(nst::fi64)replicated.size());
                for(auto r = ok.begin(); r != ok.end(); ++r) {
                    try {
                        (*r)->rollback();
                    } catch (std::exception &e) {
                        dbg_print("error closing transaction %s", e.what());
                        (*r)->close();
                    }
                }
                return false;
            }else{
                return true;
            }
        }
        /// start as many replicants as possible
        /// return false if too many failed
        template<typename version_storage_type_ptr>
        bool write_replicants(const version_storage_type_ptr& source){
            if(replicated.empty()) return true;
            if(!check_open_replicants(replicated))
                return false;
            spaces::replication_clients ok;
            for(auto r = replicated.begin(); r != replicated.end(); ++r){
                if((*r)->is_open()){
                    try{
                        source->replicate(*r);
                        ok.push_back((*r));
                    }catch(std::exception& e){
                        dbg_print("error replicating transaction %s", e.what());
                    }


                }


            }
            if(ok.size() < replicated.size() / 2){ /// at least half must succeed (democratic)
                wrn_print("too few replicants could replicate their data %lld of %lld",(nst::fi64)ok.size() ,(nst::fi64)replicated.size());
                for(auto r = ok.begin(); r != ok.end(); ++r){
                    try{
                        if((*r)->is_open()) {
                            (*r)->rollback(); /// TODO: rolback as many of them as possible
                            (*r)->close();
                        }
                    }catch(std::exception& e){
                        dbg_print("error committing transaction %s", e.what());
                        (*r)->close();
                    }

                }

                return false;
            }else{
                return true;
            }
        }
        /// start as many replicants as possible
        /// return false if too many failed
        bool read_replicants
        (   nst::buffer_type& out
        ,   nst::version_type& version
        ,   const nst::stream_address & resource
        ){
            if(replicated.empty()) return true;
            if(!check_open_replicants(replicated))
                return false;
            version_count.clear();
            version_data.clear();
            for(auto r = replicated.begin(); r != replicated.end(); ++r){
                if((*r)->is_open()){
                    try{
                        nst::buffer_type data;
                        ///std::tuple<bool,nst::version_type,nst::buffer_type>
                        auto result = (*r)->get(resource);
                        if(std::get<0>(result)){
                            nst::version_type version = std::get<1>(result);
                            if(version_data.count(version) == 0){
                                version_data[version] = std::get<2>(result);
                            }
                            version_count[version]++;
                        }


                    }catch(std::exception& e){
                        dbg_print("error retrieving resource %s", e.what());
                    }
                }
            }
            /// read from youngest to oldest version - use the first one (resource) that
            /// is contained by at least half the nodes - it should usually not take too long
            ///
            for(auto vc = version_count.rbegin(); vc != version_count.rend(); ++vc){
                if(vc->second >= replicated.size() / 2){
                    version = vc->first;
                    out = version_data[version];
                    return true;
                }
            }

            return false;
        }
        /// start as many replicants as possible
        /// return false if too many failed
        bool commit_replicants(){
            if(replicated.empty()) return true;
            if(!check_open_replicants(replicated))
                return false;
            spaces::replication_clients ok;
            for(auto r = replicated.begin(); r != replicated.end(); ++r){
                if((*r)->is_open()){
                    (*r)->commit();
                    ok.push_back((*r));
                }


            }
            if(ok.size() < replicated.size() / 2){ /// at least half must succeed (democratic)
                wrn_print("too few replicants could commit %lld of %lld",(nst::fi64)ok.size() ,(nst::fi64)replicated.size());
                for(auto r = ok.begin(); r != ok.end(); ++r){
                    try{
                        if((*r)->is_open()) {
                            (*r)->rollback(); /// TODO: rolback as many of them as possible
                            (*r)->close();
                        }
                    }catch(std::exception& e){
                        dbg_print("error committing transaction %s", e.what());
                        (*r)->close();
                    }

                }

                return false;
            }else{
                return true;
            }
        }
        bool rolback_replicants(){
            if(replicated.empty()) return true;
            if(!check_open_replicants(replicated))
                return false;
            spaces::replication_clients ok;
            for(auto r = replicated.begin(); r != replicated.end(); ++r){
                if((*r)->is_open()){
                    (*r)->rollback();
                    ok.push_back((*r));
                }


            }
            if(ok.size() < replicated.size() / 2){ /// at least half must succeed (democratic)
                wrn_print("too few replicants could commit %lld of %lld",(nst::fi64)ok.size() ,(nst::fi64)replicated.size());
                for(auto r = ok.begin(); r != ok.end(); ++r){
                    try{
                        if((*r)->is_open()) {
                            (*r)->close();
                        }
                    }catch(std::exception& e){
                        dbg_print("error closing transaction %s", e.what());
                        (*r)->close();
                    }

                }

                return false;
            }else{
                return true;
            }
        }
        nst::u64 get_max_address() const {
            if(replicated.empty()) return 0ull;
            if(!check_open_replicants(replicated))
                return 0ull;
            spaces::replication_clients ok;
            nst::u64 result = 0;
            nst::u32 count = 0;
            for(auto r = replicated.begin(); r != replicated.end(); ++r){
                if((*r)->is_open()){
                    result = std::max<nst::u64>(result,(*r)->max_block_address());
                    ++count;
                }
            }
            if(count < replicated.size() / 2){ /// at least half must succeed (democratic)
                wrn_print("too few replicants could find the data %lld of %lld",(nst::fi64)ok.size() ,(nst::fi64)replicated.size());
                return 0ull;
            }else{
                return result;
            }
        }
        bool contains(nst::u64 what){
            if(replicated.empty()) return false;
            if(!check_open_replicants(replicated))
                return false;
            spaces::replication_clients ok;
            nst::u32 count = 0;
            for(auto r = replicated.begin(); r != replicated.end(); ++r){
                if((*r)->is_open()){
                    if((*r)->contains(what)){
                        ++count;
                    }
                }
            }
            if(count < replicated.size() / 2){ /// at least half must succeed (democratic)
                wrn_print("too few replicants could find the data %lld of %lld",(nst::fi64)ok.size() ,(nst::fi64)replicated.size());
                return false;
            }else{
                return true;
            }
        }
    };
    typedef std::shared_ptr<replication_control> replication_control_ptr;
}


#endif //SPACES_REPLICATION_H
