#include "Poco/Mutex.h"
#include "system_timers.h"
#include <stx/storage/types.h>
#include <iostream>
#include <stx/storage/pool.h>
#include <storage/transactions/abstracted_storage.h>
typedef Poco::ScopedLockWithUnlock<Poco::Mutex> syncronized;
static Poco::Mutex& get_stats_lock(){
	static Poco::Mutex _c_lock;
	return _c_lock;
}
extern stx::storage::allocation::pool allocation_pool;
extern stx::storage::allocation::pool buffer_allocation_pool;
extern bool stx::memory_low_state;
namespace stx{
namespace storage{
	class low_resource_timer{
		class timer_worker : public Poco::Runnable{
		private:
			bool started;
			bool stopped;
			u64 timer_val;
		public:
			timer_worker() : timer_val(0),started(false),stopped(true){
			}
			void cancel(){
				started = false;
			}
			bool is_started() const {
				return started;
			}
			void wait_start(){
				while(!started){
					try{
						Poco::Thread::sleep(50);
					}catch(Poco::Exception &){
						inf_print("stop interrupted");
					}
				}
			}
			void stop(){
				cancel();
				while(!stopped){
					try{
						Poco::Thread::sleep(50);
					}catch(Poco::Exception &){
						inf_print("stop interrupted");
					}
				}
			}
			void run(){
				stopped = false;
				started = true;
				Poco::Thread::sleep(1000);
				timer_val = os::millis();
				try{
					while(is_started()){
						Poco::Thread::sleep(10);
						double total_mb = ((double)(allocation_pool.get_total_allocated()+buffer_allocation_pool.get_total_allocated())/(1024.0*1024.0));
						double max_alloc = ((double)allocation_pool.get_max_pool_size())/(1024.0*1024.0);
						double max_buf_alloc = buffer_allocation_pool.get_max_pool_size()/(1024.0*1024.0);
                        if(allocation_pool.is_near_depleted() || buffer_allocation_pool.is_near_depleted()){
                            if(!::stx::memory_low_state){

                                //std::cout << "switching to low state" << std::endl;
                            }
							inf_print(" resources: %.3f",total_mb);
							::stx::memory_low_state = true;
							//stored::reduce_all();
							while(is_started() && allocation_pool.is_near_factor(0.75)) {
								Poco::Thread::sleep(10);
								//stored::reduce_all();
							}
                            ::stx::memory_low_state = false;
                        }else{
                            ::stx::memory_low_state = false;
                        }
						timer_val = os::millis();
					}
				}catch(Poco::Exception &){
					dbg_print("timer interrupted");
				}
				dbg_print("timer is canceled");
				stopped = true;
			}
			u64 get_timer() const {
				return this->timer_val;
			}
		};
	private:
		Poco::Thread timer_thread;
		timer_worker worker;
	public:
		low_resource_timer() : timer_thread("spaces:timer_thread"){
		    dbg_print(" starting resource thread ");
            try{
				timer_thread.start(worker);
				//worker.wait_start();
			}catch(Poco::Exception &ex){
				err_print("Could not start timer thread : %s\n",ex.name());
			}
		}
		u64 get_timer() const {
			return this->worker.get_timer();
		}
		~low_resource_timer(){
			try{
				if(worker.is_started()){
					worker.stop();
				}

			}catch(Poco::Exception &ex){
				err_print("Could not stop timer thread : %s\n",ex.name());
			}
		}
	};
	static low_resource_timer lrt;
	u64 get_lr_timer(){
		return lrt.get_timer();
	}
	Poco::Mutex& get_single_writer_lock(){
		static Poco::Mutex swl;
		return swl;
	}
	std::string data_directory = ".";

	std::string remote = "";

	long long total_use = 0;

	long long buffer_use = 0;

	long long col_use = 0;

	long long stl_use = 0;

	Poco::UInt64 ptime = os::millis() ;
	Poco::UInt64 last_flush_time = os::millis() ;
	extern void add_total_use(long long added){
		syncronized l(get_stats_lock());
		if(added < 0){
			err_print("adding neg val");
		}
		total_use += added;
	}
	extern void remove_total_use(long long removed){
		syncronized l(get_stats_lock());
		if(removed > total_use){
			err_print("removing more than added");
		}
		total_use -= removed;
	}

	extern void add_buffer_use(long long added){
		if(added==0) return;
		syncronized l(get_stats_lock());
		total_use += added;
		buffer_use += added;
	}

	extern void remove_buffer_use(long long removed){
		syncronized l(get_stats_lock());
		total_use -= removed;
		buffer_use -= removed;
	}

	extern void add_col_use(long long added){
		syncronized l(get_stats_lock());
		total_use += added;
		col_use += added;
	}

	extern void remove_col_use(long long removed){
		syncronized l(get_stats_lock());
		total_use -= removed;
		col_use -= removed;
	}
	extern void add_stl_use(long long added){
		syncronized l(get_stats_lock());
		total_use += added;
		stl_use += added;
	}

	extern void remove_stl_use(long long removed){
		syncronized l(get_stats_lock());
		total_use -= removed;
		stl_use -= removed;
	}

}
}
