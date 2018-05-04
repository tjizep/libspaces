#pragma once
#include <string>
#include <iostream>
#include <storage/spaces/key.h>
#include <stx/btree_map.h>
#include <storage/transactions/abstracted_storage.h>
#define STORAGE_NAME "spaces.data"
#ifdef __LOG_NAME__
#undef __LOG_NAME__
#endif
#define __LOG_NAME__ "DBMS"
namespace stx {
    namespace storage {
        extern long long total_use;
    }
}
namespace spaces{

	class dbms {
	public:

		stored::abstracted_storage storage;
		typedef stx::btree_map<key, record, stored::abstracted_storage, std::less<key>> _Set;
	private:
		_Set set;
		nst::i64 id;
		nst::i64 start_id;
		bool is_reader;
		static const nst::stream_address ID_ADDRESS = 8;
	public:
		bool reader() const {
			return  this->is_reader;
		}
		dbms(const std::string& name,bool is_reader)
        :   storage(name)
        ,   set(storage)
        ,   id(1)
        ,   is_reader(is_reader) {

            storage.rollback();
		}
		~dbms() {
			
			dbg_print("stopping dbms...%s", storage.get_name().c_str());
			storage.rollback();
			dbg_print("ok %s", storage.get_name().c_str());
		}

		const std::string& get_name(){
			return storage.get_name();
		}

		_Set& get_set() {
			return set;
		}
		nst::u64 gen_id(){
		    return id++;
		}
		void begin() {
			if (!this->storage.is_transacted()) {
				stored::abstracted_tx_begin(is_reader, false, storage, set);
                if(!storage.get_boot_value(id,ID_ADDRESS)){
                    id = 1;
                }
				start_id = id;
			}
		}
		void rollback(){
            if (this->storage.is_transacted()) {
				dbg_print("rollback  %s on %s",this->storage.get_version().toString().c_str(),storage.get_name().c_str());
                this->storage.rollback();
            }
		}
        void add_replicant(const std::string& address, nst::u16 port){
		    storage.add_replicant(address,port);
		}
		void check_resources(){

		}
		void commit() {

			if (this->storage.is_transacted()){

				try{
					if(this->is_reader){
						dbg_print("commit readonly rollback %s on %s",nst::tostring(this->storage.get_version()),storage.get_name().c_str());
						this->storage.rollback();
					}else{
						dbg_print("commit write save id [%lld] %s on %s",(nst::fi64)id,nst::tostring(storage.get_version()),storage.get_name().c_str());
						if(start_id != id){
							storage.set_boot_value(id, ID_ADDRESS);
							start_id = id;
						}
						dbg_print("commit final %s on %s",nst::tostring(storage.get_version()),storage.get_name().c_str());
						this->set.flush_buffers();
						this->storage.commit();
						if(this->storage.is_local_writes()){
							dbg_print("commit synch. to io %s on %s",nst::tostring(storage.get_version()),storage.get_name().c_str());
							nst::journal::get_instance().synch();
						}
					}
				}catch (...){
					err_print("error during commit");
				}



			}

		}
	};
	extern std::shared_ptr<dbms> get_writer();
	static std::shared_ptr<dbms> create_reader(){
		return std::make_shared<dbms>(STORAGE_NAME,true);
	}
}
#undef __LOG_NAME__
#define __LOG_NAME__ __LOG_SPACES__