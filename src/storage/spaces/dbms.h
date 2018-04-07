#pragma once
#include <string>
#include <iostream>
#include <storage/spaces/key.h>
#include <stx/btree_map.h>
#include <abstracted_storage.h>

namespace stx {
    namespace storage {
        extern long long total_use;
    }
}
namespace spaces{
	class dbms {
	public:

		stored::abstracted_storage storage;
		typedef stx::btree_map< key, record, stored::abstracted_storage, std::less<key>> _Set;
	private:
		_Set set;
		nst::i64  id;
		static const nst::stream_address ID_ADDRESS = 8;
	public:
		dbms(const std::string &name) : storage(name), set(storage), id(1) {
			storage.begin(true);
			if(!storage.get_boot_value(id,ID_ADDRESS)){
				id = 1;
			}

			allocation_pool.set_max_pool_size(1024*1024*1024*6ull);
		}
		~dbms() {
			
			inf_print("stopping dbms...%s", storage.get_name().c_str());		
			storage.rollback();
			inf_print("ok %s", storage.get_name().c_str());			
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
				stored::abstracted_tx_begin(false, false, storage, set);
                storage.get_boot_value(id,ID_ADDRESS);
			}
				
		}
		void check_resources(){

		}
		void commit() {

			if (this->storage.is_transacted()){
				set.flush_buffers();
				storage.set_boot_value(id, ID_ADDRESS);
				this->storage.commit();
				nst::journal::get_instance().synch();
			}


			this->storage.begin(true);
		}
	};
}