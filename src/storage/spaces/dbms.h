#pragma once
#include <string>
#include <iostream>
#include <mutex>
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
	class dbms_memmanager;
	class dbms {
	public:


		typedef stx::storage::allocation::pool_alloc_tracker<key> _KeyAllocator;
		typedef stx::btree_map<key, record, stored::abstracted_storage, std::less<key>, _KeyAllocator> _Set;
	private:
		std::recursive_mutex resource_x;
		stored::abstracted_storage storage;
		_Set set;
		nst::i64 id;
		nst::i64 start_id;

		const bool is_reader;
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
			
			dbg_print("stopping dbms as a %s...%s", this->is_reader ? "reader" : "writer",storage.get_name().c_str());
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
				resource_x.lock();
				stored::abstracted_tx_begin(is_reader, false, storage, set);
                if(!storage.get_boot_value(id,ID_ADDRESS)){
                    id = 1;
                }std::mutex m;
				start_id = id;
			}
		}
		void rollback(){
            if (this->storage.is_transacted()) {
				dbg_print("rollback  %s on %s",this->storage.get_version().toString().c_str(),storage.get_name().c_str());
                this->storage.rollback();
                resource_x.unlock();
            }

		}
        void add_replicant(const std::string& address, nst::u16 port){
		    storage.add_replicant(address,port);
		}
		void check_resources(){

		}
		void check_set_resources(){
			if (this->storage.is_transacted()) {
				return;
			}
            if(resource_x.try_lock()){
				//dbg_print("dbms reducing use from memory manager");
				stored::abstracted_tx_begin(true, false, storage, set);
				set.check_use();
				this->storage.rollback();
				//stx::storage::allocation::print_allocations();
				resource_x.unlock();
            }
		}
		bool is_transacted() const {
			return this->storage.is_transacted();
		}
		bool commit() {
			bool r = false;
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
					r = true;
				}catch (const std::exception& e){
					err_print("FATAL error during commit [%s]",e.what());
					resource_x.unlock();
					throw e;
				}
				resource_x.unlock();

			}

			return r;
		}
    public:
		typedef std::shared_ptr<dbms> ptr;
        typedef dbms* ref;
	};
	static spaces::dbms::ptr writer;

	class dbms_memmanager{
	private:
		typedef std::unordered_map<ptrdiff_t, spaces::dbms::ptr> _Active;

		bool canceling;
		bool started;

		_Active active;
		std::shared_ptr<std::thread> checker;
		std::recursive_mutex manage_x;
    private:
        void remove(const spaces::dbms::ptr& dbms_ptr){
            dbg_print("collecting dbms %s", dbms_ptr->get_name().c_str());
            active.erase((ptrdiff_t)(void *)dbms_ptr.get());
        }
        void manage_instances(){
            for (auto a = active.begin(); a != active.end(); ++a) {
                spaces::dbms::ptr dbms = a->second;
                if(dbms.use_count()==2 && dbms->reader()){
                    dbg_print("removing reader from management");
                    remove(dbms);
                }
            }
        }
        void manage_memory(){
			if(buffer_allocation_pool.is_near_depleted()){
				stored::reduce_all();
			}
			/// allocation_pool.is_near_factor(0.75)
            if(allocation_pool.is_near_depleted()) {
				::stx::memory_low_state = true;
                for (auto a = active.begin(); a != active.end(); ++a) {
                    spaces::dbms::ptr dbms = a->second;
                    dbms->check_set_resources();
                }
            }else{
				::stx::memory_low_state = false;
			}
        }
	public:
		dbms_memmanager()
		:   canceling(false)
		,	started(false)
		{
			dbg_print("dbms manager thread initialized");

			checker = std::make_shared<std::thread>(&dbms_memmanager::check_dbms,this);
			while(!started){
				std::this_thread::sleep_for (std::chrono::milliseconds(10));
			}


		}
		void add(const spaces::dbms::ptr& dbms_ptr){
        	if(canceling)
				return;
            std::lock_guard<std::recursive_mutex> lock(manage_x);
			active[(ptrdiff_t)(void *)dbms_ptr.get()] = dbms_ptr;
		}

		void check_dbms(){
			dbg_print("start dbms checker");
			started = true;
			try{
                while(!canceling){
                    {
                        std::lock_guard<std::recursive_mutex> lock(manage_x);
                        manage_instances();
                        manage_memory();
                    }

                    std::this_thread::sleep_for (std::chrono::milliseconds(100));
                }
			}catch (...){
			    err_print("dbms memory management error");
			}



			dbg_print("finish dbms checker");
			started = false;
		}
		~dbms_memmanager(){
        	dbg_print("~dbms_memmanager");
			canceling = true;

			while(started){
				std::this_thread::sleep_for (std::chrono::milliseconds(10));
			}
			{
				std::lock_guard<std::recursive_mutex> lock(manage_x);
				active.clear();
			}

			try
			{

				if(checker->joinable())
					checker->join();
			}catch(std::exception& e){
				err_print("error joining mmghre thread [%s]", e.what());
			}
		}

	};

	extern dbms::ptr get_writer();
	extern dbms::ptr create_reader();
}
#undef __LOG_NAME__
#define __LOG_NAME__ __LOG_SPACES__