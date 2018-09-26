// spaces.cpp : Defines the exported functions for the DLL application.
//
#ifdef __cplusplus
extern "C" {
#include <lua.h>

}
#endif
#if LUA_VERSION_NUM == 501
#include "lua_xt.h"
#endif
#include <storage/spaces/data_type.h>

#include <storage/interface/space_lua.h>

#include <storage/network/replication.h>

#include <storage/transactions/abstracted_storage.h>
extern void start_storage();
extern void stop_storage();

DEFINE_SESSION_KEY(SPACES_SESSION_KEY);
typedef spaces::session_t session_t;
typedef spaces::spaces_iterator<session_t::_Set> lua_iterator_t;


static size_t breaks = 0;
static void debug_break(){
	if(nst::storage_debugging){
		++breaks;
	}
}
static std::string& lua_tostdstring(lua_State *L, int index){
	thread_local std::string result;
	result.clear();

	size_t l = 0;
	if(lua_isstring(L,index)){
		const char * s = luaL_checklstring(L,index,&l);

		if(s!=NULL){
			result.append(s,l);
		}
	}

	return result;
}

namespace spaces{
	typedef state_cache<lua_State,session_t,spaces::space> state_cache_type;
	static state_cache_type states;

	space * get_space(lua_State *L, int at = 1) {
		space* r =  is_space_<space>(L,at);
		if(r==nullptr){
			luaL_error(L, "no key associated with table of type %s", SPACES_LUA_TYPE_NAME);
		}
		return r;
	}
	state_cache_type::variables_type::sessions_type& get_sessions(lua_State *L){
		return states.get_sessions(L);
	}
	void add_space(lua_State *L, nst::lld pt, spaces::space* r){
		lua_Debug ar;
		lua_getstack(L, 1, &ar);
		lua_getinfo(L, "lS", &ar);
		std::cout << "add space line:" << ar.currentline << " : " << ar.source << std::endl;

	}
	void remove_session(lua_State *L,const std::string& storage){
		states.remove_session(L,storage);

	}
	/**
	 * destroy a space potentialy cleaning up other resources
	 * @param L
	 */
	void close_space(lua_State *L, space* p) {
		p->~space();
	}
	session_t::lua_iterator*  get_iterator(lua_State *L, int at = 1) {
		auto * i = err_checkudata<session_t::lua_iterator>(L, SPACES_ITERATOR_LUA_TYPE_NAME, at);
		if(i==nullptr || !i->has_session()){
			luaL_error(L, "invalid connection or session associated with iterator %s", SPACES_LUA_TYPE_NAME);
		}
		if(!i->get_session()->reader()){
			i->recover();
		}
		return i;
	}
}
static int  l_space_close(lua_State *L) {
	return 0;
}
static int l_serve_space(lua_State *L) {
    nst::u32 port = spaces::DEFAULT_PORT;
    if(lua_isnumber(L,1)){
        port = lua_tointeger(L,1);
    }
    spaces::block_replication_server server(port);
	server.run();

    return  0;
}
static int l_seed_space(lua_State *L) {
    try{
        if (lua_isstring(L, 1) && lua_isnumber(L, 2)) {
			const char * ip = lua_tostring(L,1);
			nst::u16 port = lua_tointeger(L,2);
			stored::get_abstracted_storage(STORAGE_NAME)->add_seed(ip,port);


		}
    }catch(std::exception& e){
        luaL_error(L,"could not connect: %s",e.what());
    }
    return 0;
}
static int l_space_local_writes(lua_State *L) {
    try{

        stored::get_abstracted_storage(STORAGE_NAME)->set_local_writes(lua_toboolean(L,1));
    }catch(std::exception& e){
        luaL_error(L,"could not set local writes: %s",e.what());
    }
	return 0;
}
static int l_space_observe(lua_State* L){
	try{

		if (lua_isstring(L, 1) && lua_isnumber(L, 2)) {
			const char *ip = lua_tostring(L, 1);
			nst::u16 port = lua_tointeger(L, 2);

			stored::get_abstracted_storage(STORAGE_NAME)->get_replication_control()->set_observer(ip,port);
		}
	}catch(std::exception& e){
		luaL_error(L,"could not set observer: %s",e.what());
	}
	return 0;
}
static int l_replicate_space(lua_State *L) {
    try{
        if (lua_isstring(L, 1) && lua_isnumber(L, 2)) {
            const char * ip = lua_tostring(L,1);
			nst::u16 port = lua_tointeger(L,2);
			stored::get_abstracted_storage(STORAGE_NAME)->add_replicant(ip,port);

        }
    }catch(std::exception& e){
        luaL_error(L,"could not connect: %s",e.what());
    }
    return 0;
}
static int l_open_session(lua_State *L) {
	const char * name = STORAGE_NAME;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L,1);
	}
	auto s = spaces::states.get_sessions(L).create_session(nullptr,name);
	spaces::push_session<session_t>(L, name, s, false);
	s->begin(true); /// will start transaction
	return 1;
}
static int l_session_open_space(lua_State *L) {
	debug_break();
	auto s = spaces::get_session<spaces::session_t>(L);
    s->begin(true); /// will start transaction
	session_t::space*  r = s->open_space(s,0);

	if (s->get_set().size() != 0) {
		s->resolve_id(r);
	}
	return 1;
}
static int l_session_read(lua_State *L) {
	debug_break();
	spaces::get_session<session_t>(L);

	return 0;
}
static int l_session_write(lua_State *L) {
	debug_break();
	auto s = spaces::get_session<session_t>(L);
	s->begin_writer();
	return 0;;
}

static int l_session_from(lua_State *L) {
	return 0;
}

static int l_session_begin(lua_State *L) {

	auto s = spaces::get_session<session_t>(L);

	s->begin(false);
	return 0;
}
static int l_session_begin_reader(lua_State *L) {
	auto s = spaces::get_session<session_t>(L);
	s->begin(true);
	return 0;
}

static int l_session_commit(lua_State *L) {
	debug_break();
	spaces::get_session<session_t>(L)->commit();
	return 0;
}

static int l_session_rollback(lua_State *L) {
	debug_break();
	session_t::ptr s = spaces::get_session<session_t>(L);
	s->rollback();
	return 0;
}

static int l_space_debug(lua_State* L) {
	debug_break();
	nst::storage_debugging = true;
	nst::storage_info = true;
	if(lua_type(L, 1) == LUA_TSTRING){
		dbg_print("---------------SPACES DEBUG START [%s]------------------------------------------",lua_tostring(L,1));
	}else{
		dbg_print("---------------------------- SPACES DEBUG START ------------------------------------------");
	}

	return 0;
}
static int l_space_quiet(lua_State* L) {
	debug_break();
	if(lua_type(L, 1) == LUA_TSTRING){
		dbg_print("---------------SPACES DEBUG END [%s]--------------------------------------------",lua_tostring(L,1));
	}else{
		dbg_print("---------------------------- SPACES DEBUG START ------------------------------------------");
	}

	nst::storage_debugging = false;
	nst::storage_info = false;
	return 0;
}
static int l_configure_space(lua_State *L) {
	debug_break();
	if (lua_isstring(L, 1)) {
		nst::data_directory = lua_tostring(L,1);
		dbg_print("storage directoryt set to %s",nst::data_directory.c_str());
	}
	return 0;
}
static int l_setmaxmb_space(lua_State *L) {
	debug_break();
	nst::u64 mmb = (nst::u64)lua_tonumber(L,1);
	dbg_print("set max mem to %lld",(nst::lld)mmb);
	allocation_pool.set_max_pool_size((1024ULL*1024ULL*mmb)*0.7f);
	buffer_allocation_pool.set_max_pool_size((1024ULL*1024ULL*mmb)*0.3f);
    return 0;
}


static const struct luaL_Reg spaces_f[] = {
	{ "open", l_open_session },
	{ "storage", l_configure_space },
    { "serve", l_serve_space},
    { "setMaxMb", l_setmaxmb_space },
    { "replicate", l_replicate_space },
    { "seed", l_seed_space },
	{ "debug", l_space_debug },
	{ "quiet", l_space_quiet },
    { "localWrites", l_space_local_writes },
	{ "observe", l_space_observe },
	{ "__gc", l_space_close },
	{ NULL, NULL } /* sentinel */
};


static int spaces_close(lua_State *L) {
	debug_break();
	spaces::space * p = spaces::get_space(L);
	spaces::close_space(L,p);
	return 0;
}
static int spaces_len(lua_State *L) {
	debug_break();
	spaces::space * p = spaces::get_space(L);
	lua_pushnumber(L, p->get_session()->len(p));

	return 1;
}

static int spaces_less_equal(lua_State *L) {

	spaces::space * l = spaces::get_space(L,1);
	spaces::space * r = spaces::get_space(L,2);
	lua_pushboolean(L, !(r->first < l->first));
	return 1;

}
static int spaces_less_than(lua_State *L) {

	spaces::space * l = spaces::get_space(L,1);
	spaces::space * r = spaces::get_space(L,2);
	lua_pushboolean(L, l->first < r->first);
	return 1;
}
static int spaces_equal(lua_State *L) {

	spaces::space * l = spaces::get_space(L,1);
	spaces::space * r = spaces::get_space(L,2);
	lua_pushboolean(L, !(r->first != l->first));

	return 1;
}

static int spaces_newindex(lua_State *L) {
	// t[k] = v will be t,k,v <-> 1,2,3
	debug_break();
	spaces::top_check tc(L,0);
	int t = lua_gettop(L);
	if(t!=3) luaL_error(L,"invalid argument count for subscript assignment");
	spaces::space k;
	spaces::space* p = spaces::get_space(L,1);
    session_t::ptr s = std::static_pointer_cast<session_t>(p->get_session());
	s->begin_writer();
	s->resolve_id(p);
	k.first.set_context(p->second.get_identity());
	s->to_space_data( k.first.get_name(), 2);

	if(lua_isnil(L,3)){
		spaces::dbg_space("erasing",k.first,k.second);
		s->erase(k.first);
	}else{
		s->to_space(k, 3);
		s->insert_or_replace(k);

	}

	return 0;
}
/**
 * resolve a route if one is flagged
 * @param s the session which will be used
 * @param value the value of the context,key,value tuple this can be replaced if the current value is a route
 * @return the original session if no route was specified
 */
static std::pair<session_t::ptr,bool> resolve_route(lua_State*L,const session_t::ptr &s,const spaces::record& value){
	if(value.is_flag(spaces::record::FLAG_ROUTE)){ /// this is a route
		std::string storage = s->map_data(value).get_value().get_sequence().std_str();
		if(s->get_name() != storage ){
			dbg_print("resolving route '%s' -> '%s' on id [%lld]",s->get_name().c_str(),storage.c_str(),(nst::lld)value.get_identity());

			auto r = spaces::states.get_sessions(L).create_session(nullptr,storage);
			if(r->get_state() == nullptr){
				dbg_print("found new session %s setting lua state",storage.c_str());
				r->set_state(L);
			}
			//r->begin(true);

			return std::make_pair(r,true);
		}
	}
	return std::make_pair(s,false);
}
/**
 * push a space or data onto the lua stack
 * if the space has an identity a space object is pushed
 * else one of the lua primitive types are pushed
 * @param L the lua state
 * @param ps the session
 * @param key the key of potential
 * @param value
 * @return 1 if something was pushed 0 otherwise
 */
static int push_space(lua_State*L, const session_t::ptr &ps,const spaces::key& key, const spaces::record& value){
	spaces::top_check tc(L,1);

	if (!value.is_flag(spaces::record::FLAG_LARGE) && value.get_identity() != 0) {
		dbg_space("push_space: ",key,value);

		auto s = resolve_route(L,ps,value);

		spaces::space *r = s.first->open_space(s.first,value.get_identity());

		r->first = key;
		r->second = value;
		spaces::dbg_space("push_space: space pushed:",r->first, r->second);
		if(s.second){// it was routed
			/// stop it from routing again
			r->second.clear_flag(spaces::record::FLAG_ROUTE);
			r->second.get_value().clear();
		}
	} else {
		ps->push_data(ps->map_data(value).get_value());
	}
	return 1;
}
/**
 * implements __index meta method
 * if the object could not be found nil is pushed
 * @param L the lua state
 * @return 1
 */
static int spaces_index(lua_State *L) {
	spaces::top_check tc(L,1);
	debug_break();
	spaces::space* p = spaces::get_space(L,1);
	session_t::ptr s = std::static_pointer_cast<session_t>(p->get_session());

	//s->get_set().check_surface_uses("spaces_index a");


	s->begin(true); /// use whatever mode is set
	spaces::space k;

	if (p->second.get_identity() != 0) {
		spaces::dbg_space("index from:",p->first,p->second);
		s->to_space_data(k.first.get_name(), 2);
		k.first.set_context(p->second.get_identity());

		//session_t::_Set::iterator i = s->get_set().find(k.first); /// a non hash optimized lookup
		const auto value = s->get_set().direct(k.first);/// a hash optimized lookup

		if (value != nullptr) { //if(i != s->get_set().end()){//
			//spaces::dbg_space("found space:",k.first,i.value());
			return push_space(L,s,k.first,*value);
            //spaces::dbg_space("found space:",k.first,i.value());
            //int r =  push_space(L,s,k.first,i.value());
			//s->get_set().check_surface_uses("spaces_index b");
			//return r;

        } else {
			lua_pushnil(L);
		}



	}
	else { /// cannot index something thats not a parent
		lua_pushnil(L);
	}
	//s->get_set().check_surface_uses("spaces_index b");
	return 1;

}
/**
 * pushes a number optionally converting a space object to a number
 * @param L the lua state
 * @param at where on the lua state the conversion will take place
 * @return 1
 */
static f8 to_number(lua_State *L, i4 at) {
	f8 r = 0.0;
	if (lua_isnumber(L, at)) {
		r = lua_tonumber(L, at);
	}
	else {

		spaces::space* p = spaces::get_space(L,at);
		if (p) {
			r = p->second.get_value().to_number();
		}
	}
	return r;
}
static int spaces_add(lua_State *L) {
	lua_pushnumber(L, to_number(L, 1) + to_number(L, 2));
	return 1;
}
static int spaces_sub(lua_State *L) {
	lua_pushnumber(L, to_number(L, 1) - to_number(L, 2));
	return 1;
}
static int spaces_neg(lua_State* L) {
	lua_pushnumber(L, -to_number(L, 1));
	return 1;
}
static int spaces_mul(lua_State* L) {
	lua_pushnumber(L, to_number(L, 1)*to_number(L, 2));
	return 1;
}
static int spaces_pow(lua_State* L) {
	lua_pushnumber(L, pow(to_number(L, 1), to_number(L, 2)));
	return 1;
}
static int spaces_div(lua_State* L) {
	lua_pushnumber(L, to_number(L, 1) / to_number(L, 2));
	return 1;
}
static int spaces_tostring(lua_State *L) {
	spaces::space* p = spaces::get_space(L,1);
	auto s = std::static_pointer_cast<session_t>(p->get_session());
	if (p->second.get_identity() != 0) {
		lua_pushfstring(L, "table: %p", p->second.get_identity());
		return 1;
	}
	umi l = 0;
	std::string value;
	if (p != nullptr) {
		s->push_data(p->second.get_value());
		return 1;
	}
	return 0;
}
static int spaces_metatable(lua_State *L) {
	lua_newtable(L);
	return 1;
}
static int spaces_type(lua_State* L) {
	lua_pushstring(L, "table");
	return 1;
}
/**
 * move the iterator one forward
 * @param L
 * @return 0
 */
static int l_pairs_iter(lua_State* L) { //i,k,v
	debug_break();

	auto *i = spaces::get_iterator(L,lua_upvalueindex(1));


	if (!i->end()) {
		spaces::top_check tc(L,2);

		dbg_print("advancing iterator: session name: '%s'",i->get_session()->get_name().c_str());
		session_t::ptr s = resolve_route(L,i->get_session(),spaces::get_data(i->get_i())).first;
		const spaces::key &k = spaces::get_key(i->get_i());
		const spaces::record& v = spaces::get_data(i->get_i());
		spaces::dbg_space("advancing iterator:",k,v);
		int r = s->push_pair(s,k,v);
		i->next();
		dbg_print("advanced iterator");
		return r;
	}
	
	return 0;
}
/**
 * set an iterator meta table to a negative position in stack
 * @param L lua state
 * @param at negative position relative to top
 */
 namespace spaces{
	 nst::lld iterator_count = 0;
 };

static void set_iter_meta(lua_State* L,int at){
	debug_break();
    if(at >= 0){
        luaL_error(L, "invalid parameter");
    }
    // set its metatable
    luaL_getmetatable(L, SPACES_ITERATOR_LUA_TYPE_NAME);
    if (lua_isnil(L, -1)) {
        luaL_error(L, "no meta table of type %s", SPACES_ITERATOR_LUA_TYPE_NAME);
    }
    if(at < 0)
        lua_setmetatable(L, at-1);
}
static int push_iterator(lua_State* L, session_t::ptr s, const spaces::space* p, const spaces::data& lower, const spaces::data& upper){

	dbg_print("iterate count %lld",spaces::iterator_count);
	spaces::key f,e ;

	f.set_context(p->second.get_identity());
	e.set_context(p->second.get_identity());
	f.set_name(lower);
	e.set_name(upper);

	// create the lua iterator

	auto * pi = s->create_iterator(s);
	pi->set_lower(s->get_set(),f);
	pi->set_upper(s->get_set(),e);

	// set its metatable
    set_iter_meta(L,-1);


	return 1;
}

static int push_iterator_closure(lua_State* L, session_t::ptr s, const spaces::space* p, const spaces::data& lower, const spaces::data& upper){

	push_iterator(L,s,p,lower,upper);

	lua_pushcclosure(L, l_pairs_iter, 1);//i

	return 1;
}
inline const spaces::data make_inf(){
	spaces::data inf;
	inf.make_infinity();
	return inf;
}
static int spaces___pairs(lua_State* L) {
	spaces::top_check tc(L,1);
	debug_break();
	spaces::space* p = spaces::get_space(L,1);
	const spaces::key& k = p->first;
	const spaces::record& v = p->second;
	spaces::dbg_space("starting iterator:",k,v);
	session_t::ptr s = std::static_pointer_cast<session_t>(p->get_session());
	auto resolved = resolve_route(L, s, p->second);
	s = resolved.first;
	if(resolved.second){
		dbg_print("starting iterator resolved to : '%s']",s->get_name().c_str());
	}
	s->begin(true); /// will start a transaction
	int r = push_iterator_closure(L, s, p, spaces::data(),make_inf());
	return r;
}

static int spaces_call(lua_State *L) {
	debug_break();
	int t = lua_gettop(L);

	int o = (t >= 4) ? 1: 0;
	spaces::space* p = spaces::get_space(L,1);
    auto s = std::static_pointer_cast<session_t>(p->get_session());
    s = resolve_route(L, s, p->second).first;
    s->begin(true); /// will start a transaction
	spaces::data lower;
	spaces::data upper = make_inf();
	if(t <= 2){
		return push_iterator(L, s, p, lower, upper);
    }
	if(t >= o + 2)
		s->to_space_data(lower, o + 2);
	if(t >= o + 3)
		s->to_space_data(upper, o + 3);
	return push_iterator_closure(L, s, p, lower, upper);


}

// meta table for spaces
const struct luaL_Reg spaces_m[] = {
	{ "__index", spaces_index },
	//{ "__intex", spaces_intex },
	{ "__newindex", spaces_newindex },
	//{ "__tostring", spaces_tostring },
	{ "__metatable", spaces_metatable },
	{ "__call", spaces_call },
	//{ "__type", spaces_type },
	//{ "__add",spaces_add },
	//{ "__sub",spaces_sub },
	//{ "__mul",spaces_mul },
	//{ "__div",spaces_div },
	//{ "__unm",spaces_neg },
	//{ "__pow",spaces_pow },
	{ "__gc", spaces_close },
	{ "__len", spaces_len },
	//{ "__le", spaces_less_equal },
	//{ "__lt", spaces_less_than },
	//{ "__eq", spaces_equal },
	{ "__pairs", spaces___pairs },
	{ NULL, NULL }/* sentinel */
};

static int l_pairs_iter_start(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
	i->start();
    return 0;
}
static int l_pairs_iter_last(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
	i->last();
    return 0;
}
static int l_pairs_iter_count(lua_State* L) {
	debug_break();
	auto *i = spaces::get_iterator(L,1);

	lua_pushinteger(L,i->count());

	return 1;

}
static int l_pairs_iter_key(lua_State* L){
	debug_break();
    auto *i = spaces::get_iterator(L,1);
    //i->get_set().check_surface_uses("l_pairs_iter_key 1");
    if(lua_gettop(L) > 1){
        long long index = lua_tonumber(L,2);
        /// use the lua index convention
        return i->get_session()->push_data(spaces::get_key(i->get_at(index-1)).get_name());
    }
    //i->get_set().check_surface_uses("l_pairs_iter_key 2");
	if (!i->end()){
       // i->get_set().check_surface_uses("l_pairs_iter_key 3");
	    int r = i->get_session()->push_data(spaces::get_key(i->get_i()).get_name());
        //i->get_set().check_surface_uses("l_pairs_iter_key 4");
        return r;
	}

	lua_pushnil(L);
	return 1;

}
static int l_pairs_iter_first_key(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
    //i->get_set().check_surface_uses("l_pairs_iter_first_key 1");
	int r = i->get_session()->push_data(spaces::get_key(i->get_first()).get_name());
    //i->get_set().check_surface_uses("l_pairs_iter_first_key 2");
    return r;

}
static int l_pairs_iter_last_key(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
	return i->get_session()->push_data(spaces::get_key(i->get_last()).get_name());

}
static int l_pairs_iter_value(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
	if(lua_gettop(L) > 1){
		long long index = lua_tonumber(L,2);
		/// use the lua index convention
		auto iter = i->get_at(index-1);
		return push_space(L, i->get_session(), spaces::get_key(iter), spaces::get_data(iter));
	}
	if (!i->end())
		return push_space(L, i->get_session(), spaces::get_key(i->get_i()), spaces::get_data(i->get_i()));

	lua_pushnil(L);
	return 1;

}
static int l_pairs_iter_pair(lua_State* L){
	debug_break();
    auto *i = spaces::get_iterator(L,1);
    if (!i->end()) {
        return i->get_session()->push_pair(i->get_session(), spaces::get_key(i->get_i()), spaces::get_data(i->get_i()));
    }else{
        lua_pushnil(L);
        lua_pushnil(L);
    }
    return 2;
}
static int l_pairs_iter_next(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
    if (!i->end()) {
        i->next();
    }
    return 0;
}
static int l_pairs_iter_move(lua_State* L){
	debug_break();
	if(lua_gettop(L) != 2) luaL_error(L,"invalid parameter to move");
	auto *i = spaces::get_iterator(L,1);
	long long index = lua_tonumber(L,2);
	i->move(index);
	return 0;
}
static int l_pairs_iter_valid(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
	lua_pushboolean(L,!i->end());
	return 1;
}
static int l_pairs_iter_empty(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
	lua_pushboolean(L,i->empty());
	return 1;
}
static int l_pairs_iter_prev(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
	if(!i->first()){
		i->previous();
	}
    return 0;
}
static int l_pairs_iter_first(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
	lua_pushboolean(L,i->first());
	return 1;
}
static int l_pairs_iter_first_value(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
	auto iter = i->get_first();
	return push_space(L, i->get_session(), spaces::get_key(iter), spaces::get_data(iter));

}
static int l_pairs_iter_last_value(lua_State* L){
	debug_break();
	auto *i = spaces::get_iterator(L,1);
	auto iter = i->get_last();
	return push_space(L, i->get_session(), spaces::get_key(iter), spaces::get_data(iter));

}
static int l_pairs_iter_close(lua_State* L){
	debug_break();
	session_t::lua_iterator *i = spaces::get_iterator(L,1);
	if(i->sentinel == i){
		i->get_session()->close_iterator(i);
		i->~lua_iterator();
		//i->get_set().check_surface_uses("l_pairs_iter_close 2");
		dbg_print("destroy iterator");
	}else{
		luaL_error(L,"attempting to destroy invalid iterator");
	}


	return 0;
}
static const struct luaL_Reg spaces_iter_f[] = {


	{ NULL, NULL } /* sentinel */
};

static const struct luaL_Reg spaces_iter_m[] = {
	{ "first",l_pairs_iter_first },
	{ "firstValue",l_pairs_iter_first_value },
	{ "start",l_pairs_iter_start },
	{ "last",l_pairs_iter_last },
	{ "lastValue",l_pairs_iter_last_value },
	{ "key",l_pairs_iter_key },
	{ "firstKey",l_pairs_iter_first_key },
	{ "lastKey",l_pairs_iter_last_key },
	{ "length",l_pairs_iter_count },
	{ "value",l_pairs_iter_value },
	{ "pair",l_pairs_iter_pair },
	{ "next",l_pairs_iter_next },
	{ "previous",l_pairs_iter_prev },
	{ "valid",l_pairs_iter_valid },
	{ "move",l_pairs_iter_move },
	{ "empty",l_pairs_iter_empty },
    { "__gc",l_pairs_iter_close },
    { NULL, NULL }/* sentinel */
};

static int l_session_close(lua_State* L) {
	debug_break();
	auto& s = spaces::get_session<session_t>(L);
	spaces::remove_session(L,s->get_name());
	s = nullptr;

	return 0;
}
static const struct luaL_Reg spaces_session_f[] = {

	{ NULL, NULL } /* sentinel */
};

static const struct luaL_Reg spaces_session_m[] =
{
	{ "open", l_session_open_space },
	{ "read", l_session_read },
	{ "write", l_session_write },
	{ "from", l_session_from },
	{ "begin", l_session_begin },
	{ "beginRead", l_session_begin_reader },
	{ "commit", l_session_commit },
	{ "rollback", l_session_rollback },
	{ "__gc",l_session_close },	
	{ NULL, NULL }/* sentinel */
};

#if 0
#endif


extern "C" int
#ifdef _MSC_VER
__declspec(dllexport)
#endif

luaopen_spaces(lua_State * L) {
	dbg_print("open lua spaces");
	start_storage();
#if LUA_VERSION_NUM == 501
	/// lua_xt is required for 5.1 pairs iterator compatibility
	lua_XT::luaopen_xt(L);
#endif
	spaces::luaopen_plib_any(L, spaces_m, SPACES_LUA_TYPE_NAME, spaces_f, SPACES_NAME);
	spaces::luaopen_plib_any(L, spaces_iter_m, SPACES_ITERATOR_LUA_TYPE_NAME, spaces_iter_f, SPACES_ITERATOR_NAME);
	lua_pop(L,1);
	spaces::luaopen_plib_any(L, spaces_session_m, SPACES_SESSION_LUA_TYPE_NAME, spaces_session_f, SPACES_SESSION_NAME);
	//spaces::luaopen_plib_any(L, spaces_recursor_m, SPACES_LUA_RECUR_NAME, spaces_recursor_f, "_spaces_recursor");
    lua_pop(L,1);

	return 1;
}
extern "C" int
#ifdef _MSC_VER
__declspec(dllexport)
#endif
luaclose_spaces(lua_State * L) {
	return 0;
}