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
	u64 get_lr_timer(){
		return 0;
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
