#pragma once
extern "C" {
	#include <lualib.h>
	#include <lauxlib.h>
	#include <lua.h>	
};
#include "spaces_session.h"

INCLUDE_SESSION_KEY(SPACES_SESSION_KEY);
INCLUDE_SESSION_KEY(SPACES_MAP_ITEM);

#define SPACES_LUA_TYPE_NAME "spacesLT" //the spaces meta table name
#define SPACES_NAME "spaces" //the spaces meta table name
#define SPACES_ITERATOR_LUA_TYPE_NAME "spacesLIT" //the spaces iterator L(I)T
#define SPACES_ITERATOR_NAME "_spaces_iterators"
#define SPACES_SESSION_LUA_TYPE_NAME "spacesS" //the spaces session 
#define SPACES_SESSION_NAME "_spaces_sessions"
#define SPACES_G "[]spaces"
#define SPACES_WRAP_KEY "__$"

namespace spaces{

	static int luaopen_plib_any(lua_State *L, const luaL_Reg *m,
					 const char *object_name, 
					 const luaL_Reg *f,
					 const char * library_name)
	{
		luaL_newmetatable(L, object_name);
		lua_pushliteral(L, "__index");
		lua_pushvalue(L, -2);
		lua_rawset(L, -3);
		luaL_register (L, NULL, m);
		luaL_register(L, library_name, f);
		return 1;
	}
	
	
	static size_t lua_tostdstring(lua_State *L,std::string& ss, int at){
		size_t l = 0;
		const char * s  = lua_tolstring(L,at,&l);	
		ss.insert(ss.end(), s,s+l);
		return l;
	}
	static void * is_udata(lua_State *L, int ud, const char* ) {
		return lua_touserdata(L, ud);
		//return luaL_checkudata(L, ud, tname);
	}
	template<typename _TypeName>
	static _TypeName *err_checkudata(lua_State *L, const char* tname, int ud = 1) {
		void *p = is_udata(L, ud, tname);
		if (p != NULL) {  /* value is a userdata? */
			return (_TypeName*)p;
		}
 		luaL_typerror(L, ud, tname);  /* else error */
		return NULL;  /* to avoid warnings */
	}
	
	template<typename _sT>
	static _sT* create_instance_from_nothing(lua_State *L) {
		size_t nbytes = sizeof(_sT);
		_sT *k = (_sT *)lua_newuserdata(L, nbytes);
		new (k) _sT();
		return k;  /* new userdatum is already on the stack */

	}
	template<typename _sT, typename _pT>
	static _sT* create_instance_from_nothing(lua_State *L, _pT param) {
		size_t nbytes = sizeof(_sT);
		_sT *k = (_sT *)lua_newuserdata(L, nbytes);
		new (k) _sT(param);
		return k;  /* new userdatum is already on the stack */

	}
	template<typename _SessionType>
	static _SessionType* create_session(lua_State *L, ptrdiff_t session_key, bool is_reader) {
		_SessionType* r = nullptr;
		lua_pushlightuserdata(L, (void *)session_key);  // push address 
		lua_gettable(L, LUA_GLOBALSINDEX);  // retrieve value 
		
		if (lua_isnil(L, -1)) {
			lua_pushlightuserdata(L, (void *)session_key);  // push address 
			r = create_instance_from_nothing<_SessionType,bool>(L,is_reader);
			r->set_state(L);
			// set its metatable 
			luaL_getmetatable(L, SPACES_SESSION_LUA_TYPE_NAME);
			if (lua_isnil(L, -1)) {
				luaL_error(L, "no meta table of type %s", SPACES_SESSION_LUA_TYPE_NAME);
				return nullptr;
			}
			lua_setmetatable(L, -2);
			lua_settable(L, LUA_GLOBALSINDEX);/// put it in globals
		}else{			
			r = err_checkudata<_SessionType>(L, "", -1);
		}
		r->check();
		r->set_mode(is_reader);
		return r;
	}
	template<typename _SessionType>
	static _SessionType* get_session(lua_State *L, ptrdiff_t session_key) {
		_SessionType* r = nullptr;
		lua_pushlightuserdata(L, (void *)session_key);  // push address
		lua_gettable(L, LUA_GLOBALSINDEX);  // retrieve value

		if (lua_isnil(L, -1)) {
			luaL_error(L,"session not created: use .read() or .write()");
			return nullptr;
		}else{
			r = err_checkudata<_SessionType>(L, "", -1);
		}

		r->check();
		return r;
	}
	
	#define tofilep(L)	((FILE **)luaL_checkudata(L, 1, LUA_FILEHANDLE))



    template<typename _SessionType>
    class lua_session : public spaces_session<_SessionType>{
	public:
		typedef spaces_session<_SessionType> session_type;
		typedef std::vector<i1> _DumpVector;
		/// despite improvements
		/// sometimes the type system still gets lost without
		/// a hint
		typedef lua_session<_SessionType> _Self;
    private:
		lua_State *L;
		_LuaKeyMap keys;
	public:
		typedef typename spaces_session<_SessionType>::_Set _Set;
		lua_session(bool reader) : L(nullptr),spaces_session<_SessionType>(reader) {

		}
		~lua_session() {
			for(auto k = keys.begin(); k != keys.end(); ++k){
				delete k->second;
			}
		}
		void close(ptrdiff_t pt) {
			std::cout << "Closing spaces key " << std::endl;
			if (keys.count(pt)) {
				delete[] keys[pt];
				keys.erase(pt);
			}
		}
		void set_state(lua_State *L) {
			this->L = L;
		}
        bool is_integer(double n) {
			return (n - (i8)n == 0);
		}
		spaces::spaces_iterator<_Set>* create_iterator() {
			return create_instance_from_nothing<spaces::spaces_iterator< _Set>>(L); /* new userdatum is already on the stack */
		}
		spaces::spaces_iterator<_Set>*  get_iterator(int at = 1) {
			spaces::spaces_iterator<_Set> * i = err_checkudata<spaces::spaces_iterator<_Set>>(L, SPACES_ITERATOR_LUA_TYPE_NAME, at);
			return i;

		}
		spaces::space* create_space_from_nothing() {
			return create_instance_from_nothing<spaces::space>(L);  /* new userdatum is already on the stack */

		}
		umi table_clustered_type(int at) {
			umi n = 0;
			if (!lua_istable(L, at)) {
				return 0;
			}
			lua_pushnil(L);
			i8 last = 0;
			while (lua_next(L, at) != 0) {
				// just set the key part from the value in lua
				umi tk = lua_type(L, -2);
				if (tk == LUA_TNUMBER) {
					double n = ::lua_tonumber(L, -2);
					if (is_integer(n) && n > 0) {
						last = (i8)n;
					}
					else {
						return 0;
					}
				}
				else {
					lua_pop(L, 2);
					return 0;
				}
				++n;
				lua_pop(L, 1);

			}

			return n;
		}

		template<typename _VectorType>
		struct vector_reader_state {
			vector_reader_state() :state(0) {};
			~vector_reader_state() {};
			i4 state;
			_VectorType v;
		};

		template<typename _VectorType>
		const char * vector_reader(void *ud, size_t *sz) {
			vector_reader_state<_VectorType> * state = (vector_reader_state<_VectorType>*)ud;

			if (state->state == 1 || state->v.empty()) {
				*sz = 0;
				return 0;
			}
			state->state = 1;
			*sz = state->v.size();
			i1* rv = (i1*) &(state->v)[0];
			return rv;
		}


		spaces::space * is_space(int at = 1) {
			if (lua_istable(L, at)) {
				ptrdiff_t pt = reinterpret_cast<ptrdiff_t>(lua_topointer(L, at));

				spaces::space * space_s = keys[pt];
				return space_s;
			}
			return nullptr;
		}
		spaces::space * get_space(int at = 1) {
			spaces::space * space = is_space(at);
			if(space==nullptr){
				luaL_error(L, "no key associated with table of type %s", SPACES_LUA_TYPE_NAME);
			}
			return space;
		}

		spaces::space* open_space(ui8 id) {
			i4 t = lua_gettop(L);
			lua_getglobal(L, SPACES_G); // so that the table can be reused if another object with the same id is accessed
			if (!lua_istable(L, -1) && lua_gettop(L) > t) {
				luaL_error(L, "spaces global reference table does not exist %s", SPACES_G);
				return nullptr;
			}
			/// TODO: shouldn't be using id + 1; its 64 bit and int can potentially be 16 bit on some architectures
			lua_rawgeti(L, -1, id + 1);
			if (lua_isnil(L, -1)) {
				lua_pop(L, 2); /// pop nil and global table of top should be the initial value again
				lua_newtable(L);
				lua_getglobal(L, SPACES_G); // so that the table can be reused if another object with the same id is accessed
				lua_pushvalue(L, -2); // copy new table
				lua_rawseti(L, -2, id + 1);
				lua_pop(L, 1); // remove global table
				ptrdiff_t pt = reinterpret_cast<ptrdiff_t>(lua_topointer(L, -1));
				spaces::space* r = new spaces::space(); // spaces::create_key_from_nothing(L);
				keys[pt] = r;
				// set its metatable 
				luaL_getmetatable(L, SPACES_LUA_TYPE_NAME);
				if (lua_isnil(L, -1)) {
					luaL_error(L, "no meta table of type %s", SPACES_LUA_TYPE_NAME);
				}
				lua_setmetatable(L, -2);

				return r;
			}
			else {
				lua_remove(L, -2);
				spaces::space* r = get_space(-1);
				return r;
			}


		}



		template<typename _VectorType>
		static int vector_writer(lua_State *L, const void* p, size_t sz, void* ud) {
			auto* v = (_DumpVector*)ud;
			v->insert(v->end(), (const _DumpVector::value_type*)p, ((const _DumpVector::value_type*)(p)) + sz);
			return 0;
		}
		void to_space_data
		(	 spaces::data& d
			, int _at = 2
		) {
			int at = _at;
			if (at < 0)
				at = lua_gettop(L) + (1 + at);//translate to absolute position
			int lt = lua_type(L, at);
			switch (lt) {
			case LUA_TNUMBER: {
				double n = ::lua_tonumber(L, at);
				d = n;
			}break;
			case LUA_TSTRING: {
				size_t l = 0;
				const char * v = ::lua_tolstring(L, at, &l);
				d.set_text(v, l);
			}break;
			case LUA_TBOOLEAN: {
				bool b = (lua_toboolean(L, at) != 0);
				d = b;
			}break;
			case LUA_TTABLE: {

				/// TODO: multi data format

			}break;
			case LUA_TUSERDATA: {

				/// TODO: multi data format

			}break;
			case LUA_TFUNCTION: {

				typename _Self::_DumpVector dv;

				lua_pushvalue(L, at);
				if (0 != lua_dump(L, vector_writer<typename _Self::_DumpVector>, &dv)) {
					lua_error(L);
				}
				lua_pop(L, 1);
				d.set_function(dv);

			}break;
			case LUA_TNIL:
			{
				d.clear();
			}break;
			default:
				luaL_error(L, "could not convert lua type to space");
				break;
			};
		}

		int push_data(const spaces::data& d) {
			switch (d.get_type()) {
			case data_type::numeric:
				lua_pushnumber(L, d.to_number());
				break;
			case data_type::boolean:
				lua_pushboolean(L, (int)d.get_integer());
				break;
			case data_type::text:
				lua_pushlstring(L, d.get_sequence().c_str(), d.get_sequence().size());
				break;
			case data_type::function:
				lua_pushnil(L);
				break;
			case data_type::infinity:
				lua_pushnumber(L, std::numeric_limits<lua_Number>::infinity());
				break;
			default:
				lua_pushnil(L);
			}
			return 1;
		}
		int push_data(const spaces::record& d) {
			return push_data(this->map_data(d).get_value());
		}

		void to_space
		(	spaces::key& d
            ,   spaces::record& r
			, int _at = 2
		) {
			int at = _at;
			if (at < 0)
				at = lua_gettop(L) + (1 + at);//translate to absolute position
			int lt = lua_type(L, at);
			switch (lt) {
			case LUA_TTABLE: {
				auto space = is_space(at);
				if(space!=nullptr){
					if (space->second.get_identity()) {
						r.set_identity(space->second.get_identity()); // a link has no identity of its own ???
					}
					break;
				}
				/// Seems like a compiler bug if this-> isn't used then all sorts of errors
				this->resolve_id(d,r,true); /// assign an identity to p (its leaves will need it)
				lua_pushnil(L);
				/// will push the name and value of the current item (as returned by closure) 
				/// on the stack
				while (lua_next(L, at) != 0) {
					spaces::space s;
					s.first.set_context(r.get_identity()); //
					to_space_data(s.first.get_name(), -2);	// just set the key part
					to_space(s.first, s.second, -1); // d becomes a parent and will receive an id if -1 is another table
										/// add new key to table
					/// Seems like a compiler bug if this-> isn't used then all sorts of errors
					this->insert_or_replace(s);
					lua_pop(L, 1);
				}

			}break;
			case LUA_TUSERDATA: {
				/// add the link here
				spaces::space * l = err_checkudata<spaces::space>(L, SPACES_LUA_TYPE_NAME, at);
				if (l == nullptr) break;
				if (l->second.get_identity()) {
					r.set_identity(l->second.get_identity()); // a link has no identity of its own ???
				}

			}break;
			case LUA_TFUNCTION: {

				typename _Self::_DumpVector dv;

				lua_pushvalue(L, at);
				if (0 != lua_dump(L, vector_writer<_Self::_DumpVector>, &dv)) {
					lua_error(L);
				}
				lua_pop(L, 1);
				r.get_value().set_function(dv);

			}break;
			case LUA_TNIL:
				/// delete space under parent using name
			{
				//d.set_context(p.get_identity());
				session_type::session.get_set().erase(d);
				/// TODO: collect garbage anyone, its wednesday
			}break;
			default:
				to_space_data(r.get_value(), at);
				break;
			};
		}
		void to_space
		(	spaces::space& d

		, 	int _at = 2
		) {
			to_space(d.first,d.second,_at);
		}
		int push_pair(const spaces::key& k,const spaces::record& v) {
			push_data(k.get_name());
			if (v.get_identity() != 0) {
				spaces::space * r = this->open_space(v.get_identity());
				r->first = k;
				r->second = v;
			}
			else {
				push_data(v.get_value());
			}
			return 2;
		}
		int push_space(const spaces::space& s) {
			return push_pair(s.first, s.second);
		}
	};
	
#if 0	
	struct spaces_session{
		static const ui4 get_sentinel(){
			return SPACES_SESSION_KEY;
		}
		static const char * get_type_name(){
			return "TSpacesSession";
		}
		i4 sent;
		DSASession * session;
		bool owner;
		spaces_session() : sent(get_sentinel()) , owner(true){
			session = session_pool::instance().out();
		}
		spaces_session(DSASession * session) : sent(get_sentinel()){
			owner = (session==NULL);
			if(owner)
				(*this).session = session_pool::instance().out();
			else
				(*this).session = session;
		}
		~spaces_session(){			
			if(owner)
				session_pool::instance().in(session);			
		}
	};
	static int l_session_close(lua_State * L){
		spaces_session * session = get_any_ud<spaces_session>(L, 1);
		if(session){
			
			session->~spaces_session();
		}
		return 0;
	}
	static const struct luaL_reg spaces_session_f[] = {
		{NULL, NULL} /* sentinel */
	};

	static const struct luaL_reg spaces_session_m[] = 
	{	{"__gc",l_session_close}
	,	{NULL, NULL}/* sentinel */
	};

	static SessionContext * get_session_context(lua_State * L){
		//_C_ASSERT(L==NULL,"invalid state");

		lua_pushlightuserdata(L, (void *)SPACES_SESSION_KEY);  // push address 
		lua_gettable(L, LUA_GLOBALSINDEX);  // retrieve value 
		spaces_session * session = get_any_ud<spaces_session>(L, -1);  // convert to system context
		if(!session){
			luaL_error(L,"spaces session was not available");
		}
		lua_pop(L,1);		
		return session->session->getSession();	
	}

	static session_space* get_session_space(lua_State *L){				
		return get_session_space(get_session_context(L));
			
	}
	static spaces_instance* get_spaces_instance(lua_State *L){	
		return get_session_space(L)->get_space();
	}
	//static spaces_instance* get_spaces_instance(SessionContext *session){	
	//	return get_session_space(session)->get_space();
	//}
	static uuid_provider& get_uuid_provider(lua_State *L){	
		return get_spaces_instance(L)->map.get_stream().get_uuid_provider();
	}
	static int open_session_context(lua_State *L,DSASession * session = NULL ){	
		_C_ASSERT(L==NULL,"invalid state");

		luaopen_plib_any(L,spaces_session_m,spaces_session::get_type_name(),spaces_session_f,spaces_session::get_type_name());
		lua_pushlightuserdata(L, (void *)SPACES_SESSION_KEY);  // push address 
		spaces_session* ss = (spaces_session*)lua_newuserdata(L, sizeof(spaces_session));
		if(!ss)
			luaL_error(L,"spaces session could not be created");
		new (ss) spaces_session(session);
		luaL_getmetatable(L, spaces_session::get_type_name());	
		lua_setmetatable(L, -2);
		// registry[&SESSION_KEY] = psystem 
		lua_settable(L, LUA_GLOBALSINDEX);    
		return 0;
	}
	static space_service * make_space(lua_State *L,const spaces_key& k,const spaces_link::ptr &context){	
		const spaces_uuid_t& identity = k.get_identity();
		i4 start = lua_gettop(L);	
		i4 gct = 0;	
		size_t nbytes = sizeof(space_struct);
		space_struct *p_space = (space_struct *)lua_newuserdata(L, nbytes);	
		if(!p_space)
			luaL_error(L,"out of memory, space could not be created");
		//new (p_space) space_struct(k, context, get_session_context(L));		
		// set its metatable 
		luaL_getmetatable(L, space_struct::get_type_name());	
		lua_setmetatable(L, -2);
		i4 end = lua_gettop(L);
		return & ( p_space->service );		
	}
	static space_service * make_space(lua_State *L,const spaces_key& k,const spaces_link::ptr &context,space_service* pcontext){	
		const spaces_uuid_t& identity = k.get_identity();
		i4 start = lua_gettop(L);	
		i4 gct = 0;	
		size_t nbytes = sizeof(space_struct);
		space_struct *p_space = (space_struct *)lua_newuserdata(L, nbytes);			
		if(!p_space)
			luaL_error(L,"out of memory, space could not be created");
		//new (p_space) space_struct(k, context, pcontext->get_session_space());		
		// set its metatable 
		luaL_getmetatable(L, space_struct::get_type_name());	
		lua_setmetatable(L, -2);
		i4 end = lua_gettop(L);
		return & ( p_space->service );		
	}

	static space_service * make_space(lua_State *L,spaces_key* k,const spaces_link::ptr &context,space_service* pcontext){	
		const spaces_uuid_t& identity = k->get_identity();
		
		size_t nbytes = sizeof(space_struct);
		space_struct *p_space = (space_struct *)lua_newuserdata(L, nbytes);			
		if(!p_space)
			luaL_error(L,"out of memory, space could not be created");
		new (p_space) space_struct(k, context, pcontext->get_session_space());		
		// set its metatable 
		luaL_getmetatable(L, space_struct::get_type_name());	
		lua_setmetatable(L, -2);
		return & ( p_space->service );		
	}
	
	

	static void table_to_mask(lua_State *L, _Mask& out, int at){
		lua_pushvalue(L,at);
		lua_pushnil(L);  		
		while (lua_next(L, -2) != 0) {	  									
			i4 lm = ::lua_tointeger(L,-1);
			out.insert(lm);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}
	template<typename _PairsT>
	void push_pair_sequence(lua_State *L, _PairsT& s,const spaces_key &k, const i4 & mask ){
		i4 tpos = lua_gettop(L);			
		_StaticSpacePair ss(s.data());			
		umi unmasked = 0;
		i4 target = mask;
		i4 ctr = 1;
		while(ss.t()!=final){
			if(target == ctr ){
				push_pair(L, ss, k, mask);// this can recurse
				
				++unmasked;
				break;
			}
			ss.next();
							
			ctr++;
		}			
	}
	

	template<typename _PairsT>
	umi push_pair_sequence(lua_State *L, _PairsT& s,const spaces_key &k,_NoMask mask){
		lua_newtable(L);
		i4 tpos = lua_gettop(L);			
		umi ctr = 1;
		_StaticSpacePair ss(s.data());
		
		while(ss.t()!=final){
			
			lua_pushinteger(L,ctr);
			push_pair(L, ss, k, mask);// this can recurse
			lua_settable(L,tpos);
			ss.next();	
			++ctr;						
			
		}//leaves a new table on top of stack	
		
		return ctr;
	}
	template<typename _PairsT>
	void push_pair_sequence(lua_State *L, _PairsT& s,const spaces_key &k, const _Mask & mask = _Mask()){
		i4 tpos = lua_gettop(L);			
		_StaticSpacePair ss(s.data());
		if(!mask.empty()){
			umi unmasked = 0;
			umi msize = mask.size();		
			i4 ctr = 1;
			if(msize == 1){
				i4 target = *(mask.begin());
				while(ss.t()!=final){
					if(target == ctr ){												
						push_pair(L, ss, k, mask);// this can recurse
						
						++unmasked;
						break;
					}						
					ss.next();						
					ctr++;
				}//leaves a new table on top of stack			
			}else{
				while(ss.t()!=final){
					if(mask.count( ctr ) > 0){						
						push_pair(L, ss, k, mask);// this can recurse						
						++unmasked;
					}
					ss.next();
					if(msize <= unmasked)
						break;
					ctr++;
				}//leaves a new table on top of stack			
			}
			if(!unmasked) 
				lua_pushnil(L);
		}else{
			_C_ERR("invalid mask");		
		}
	}
	class default_generator{
		
	public:
		default_generator(){};
		~default_generator(){};

		spaces_key operator()(lua_State *L, const spaces_key &k, const i1 * _t) const {			
			spaces_key item;
			string name = "space";
			item.add(name,ipKey());
			item.set_context(k.get_p_identity());
			spaces_uuid_t identity;
			identity.from_buffer(_t);
			item.set_identity(get_uuid_provider(L).get(identity));				
			return item;
		}

	};

	template<typename _PairsT, typename _MaskT, typename _KeyGenerator >
	i4 push_pair(lua_State *L, _PairsT& s,const spaces_key &k, const _MaskT & mask, const _KeyGenerator & generate ){
		i4 ctr = 1;
		switch(s.t()){
		case final: lua_pushinteger(L,0); break;	
		case nil: lua_pushinteger(L,0); break;
		case integer8:lua_pushinteger(L,s.cast<i1>()); break;
		case integer16: lua_pushinteger(L,s.cast<i2>()); break;
		case integer32:lua_pushinteger(L,s.cast<i4>()); break;
		case integer64: lua_pushinteger(L,s.cast<i8>()); break;
		case real: lua_pushnumber(L,s.cast<f8>()); break;
		case boolean: lua_pushboolean(L, s.cast<bool>() ? -1:0); break;
		case pair_sequence:{
			
			push_pair_sequence(L,s,k,mask);
			
		}
		break;
		case char_sequence8:
		case char_sequence:		{
			const i1 * _t = (const i1*)(*s);
			const i1 * _ts = _t;			
			++_t;
			switch(*_ts){
				case string_types::string: lua_pushlstring(L,_t,s.size()-1); break;						
				case string_types::string_element: lua_pushlstring(L,_t,s.size()-1); break;						
				//TODO: dunno how to handle this yet
				
				case string_types::file: lua_pushnil(L); break;
				case string_types::function: lua_pushlstring(L,_t,s.size()-1); break;				
				case string_types::table: 
				case string_types::space:					
					make_space(L,generate(L, k, _t),NULL);
				break;
				default: 
					_C_ERR("unknown string type encountered"); 
					break;
			}
			
		break;					}
		default: 			
			CVERR("unknown type encountered %ld\n",s.t()); 			
			break;
		}
		return ctr;
	}
	template<typename _PairsT, typename _MaskT>
	i4 push_pair(lua_State *L, _PairsT& s,const spaces_key &k, const _MaskT & mask){
		default_generator generator;
		return push_pair(L, s, k, mask, generator);
	}
	
	
	
	static i4 push_key(lua_State *L,const spaces_key &k,ipKey what){				
		i4 r = push_pair(L, *k.get(what), k, _NoMask());					
		return 1;
	}	
	static i4 push_key(lua_State *L,const spaces_key &k,ipValue what){				
		i4 r = push_pair(L, *k.get(what), k, _NoMask());					
		return 1;
	}	
	static i4 push_key(lua_State *L,const spaces_key &k){
				
		return push_key(L, k, ipKey());
	}	
	template<typename _MaskT, typename insertion_pos>
	static i4 push_key(lua_State *L,const spaces_key &k, const _MaskT& mask,insertion_pos what){
		const insertion_pos::T* pairs = k.get(what);			
		i4 r = push_pair(L,*pairs,k,mask);					
		return 1;
	}
	static i4 push_key(lua_State *L,const spaces_key &k, const i4& mask){
		return push_key(L, k, mask, ipKey());
	}
	static bool is_space(lua_State *L, int at = 1){
		return is_udata<space_struct>(L,at)!=NULL;
	}
	static spaces_instance * get_space_instance(lua_State *L, int at = 1){	
		return get_space(L, at)->get_spaces_instance();	
	}
	template<typename _VectorType>
	struct vector_reader_state{
		vector_reader_state():state(0){};
		~vector_reader_state(){};
		i4 state;
		_VectorType v;
	};
	template<typename _VectorType>
	static const char * vector_reader(lua_State *L, void *ud, size_t *sz){
		vector_reader_state<_VectorType> * state = (vector_reader_state<_VectorType>*)ud;
	
		if(state->state==1||state->v.empty()){ 
			*sz=0;
			return 0;
		}
		state->state=1;
		*sz = state->v.size();
		i1* rv =  (i1*) &(state->v)[0];
		return rv;
	}
	#define tofilep(L)	((FILE **)luaL_checkudata(L, 1, LUA_FILEHANDLE))
	template<typename _VectorType>
	static int vector_writer(lua_State *L, const void* p, size_t sz, void* ud){
		_VectorType* v = (_VectorType*)ud;
		v->insert(v->end(),(const _VectorType::value_type*)p,((const _VectorType::value_type*)(p))+sz);
		return 0;
	}
	typedef vector<ui1> _DumpVector;
	
	static void lua_array_to_buffer
	(	lua_State *L
	,	int at 
	,	spaces_key& k
	){
		if(at < 0){
			_C_ERR("invalid stack index for conversion");
		}
		lua_pushvalue(L,at);
		lua_pushnil(L);  		
		while (lua_next(L, -2) != 0) {	  			
			size_t len = 0;
			const ui1* v = (const ui1*)::lua_tolstring(L,-2,&len);		
			k.add(v, len, ipValue(),string_types::string_element);
			v = (const ui1*)::lua_tolstring(L,-1,&len);		
			k.add(v, len, ipValue(),string_types::string_element);
			lua_pop(L, 1);
		}
		lua_pop(L,1);
	}
	
	static umi table_size(lua_State *L, int at){
		umi n =0 ;
		lua_pushnil(L);  	
		while (lua_next(L, at) != 0) {	  		
			// just set the key part from the value in lua			
			++n;
			lua_pop(L, 1);
			
		}
		return n;
	}
	template<typename insertion_pos>
	static void to_space_key
	(	lua_State *L
	,	space_service * pspace
	,	spaces_key& k
	,	insertion_pos ip
	,	interpretation::Enum table2
	,	int _at = 2
	,	bool is_indexed = true
	){
		int at = _at;
		if(at < 0)
			at = lua_gettop(L) + (1+at);//translate to absolute position
		int lt = lua_type(L, at);
		switch(lt){							
		case LUA_TNUMBER:{
				double n = ::lua_tonumber(L, at);
				if(n - (i8)n != 0){
					k.add((f8)n,ip);
				}else{
					k.add((i8)n,ip);
				}				
				if(table2 == interpretation::deep){
					//k.clear_identity();
				}
			}
			break;
		case LUA_TSTRING:{
				size_t len = 0;
				const char * v = ::lua_tolstring(L,at,&len);			
				k.add(v, len, ip, string_types::string); //define string meta type
				if(table2 == interpretation::deep){
					//k.clear_identity();
				}
			}
			break;
		case LUA_TBOOLEAN:{
			bool b = (lua_toboolean(L, at)!=0);
			k.add(b,ip);
						  }
			break;
		case LUA_TTABLE:{				
			//lua_array_to_buffer(L, at, k);
			//spaces_key n  ;			
			umi n = table_clustered_type(L,at); 
			if(n>0 ){
				//umi n = table_size(L, at); 
				vector<spaces_key> &keys = pspace->get_session_space()->_keys;				
				umi c = 0;				
				keys.resize(n);
				lua_pushnil(L);  		
				while (lua_next(L, at) != 0) {	  		
					// just set the key part from the value in lua
					//keys[c].clear();
					//to_space_key(L, pspace, keys[c++], ip, table2, -2, false);	
					keys[c].clear();
					to_space_key(L, pspace, keys[c++], ip, table2, -1, false);	
					lua_pop(L, 1);
				}
				k.set_multi(keys, ip);
			}else{
				lua_pushnil(L);  		
				while (lua_next(L, at) != 0) {	  			
					if(table2 == interpretation::flat){
						to_space_key(L,pspace, k, ipKey(), interpretation::flat, -1);//only interested in the values not the keys
					}else{
				
						//if(lua_istable(L, -1)) 	{
						//	spaces_key n ; //= *(pspace->get_spaces_instance()->map.allocate());
						//	n.set_context(k.get_p_identity()); //	
						//	n.set_identity(pspace->provide_new());
						//	to_space_key(L, pspace, n, ipKey, interpretation::flat, -2, false);	// just set the key part
						//	to_space_key(L, pspace, n, ipValue, interpretation::deep, -1, false);	// set the values if any
						//	pspace->add_relationship(k,is_indexed,n);
						//	//n.clear();
						//}else {
							interpretation::Enum fdeep = lua_istable(L, -1) ? interpretation::deep:interpretation::flat;
							spaces_key  *sk = pspace->get_spaces_instance()->map.allocate();							
							to_space_key(L, pspace, *sk, ipKey(), interpretation::flat, -2, false);	// just set the key part							
							to_space_key(L, pspace, *sk, ipValue(), fdeep, -1, false); // set the values if any
							pspace->add_relationship(k,is_indexed,sk); 
							
						//}										
					}
					lua_pop(L, 1);
				}
				if(table2 == interpretation::deep){
					k.add("", 1, ip, string_types::table); //define table meta type
				}
			}	
			
		}
		break;
		case LUA_TUSERDATA:{
			
			space_service * spi = &(get_any_ud_if<space_struct>(L,at)->service);
			if(spi){
				const spaces_key& right = spi->get_current();
				if(right.get_meta_char() == string_types::table){
					k.set_identity(right.get_p_identity());
					k.add("",1,ip,string_types::table);
					if(table2 == interpretation::deep)
						k.set_context(pspace->get_identity());
				}else if(right.get_meta_char()==string_types::function){					
					k = right;
					k.set_identity(pspace->provide_new());
					if(table2 == interpretation::deep)
						k.set_context(pspace->get_identity());					
				}else {					
					k = right;					
					if(table2 == interpretation::deep){
						k.set_context(pspace->get_identity());
					}else{ 
						//TODO only the value of the key needs to be copied
						_C_ERR("case isnt currently handled properly");
					}
					//k.clear_identity();
				}				
				break;
			}	
			FILE** f = tofilep(L);
			if(f && *f){
				//fgets(
				k.add(LUA_FILEHANDLE,sizeof(LUA_FILEHANDLE),ip,string_types::file);
				//TODO: create data storage
				//TODO: copy file into data storage
			}
			
		
		}
		break;
		case LUA_TFUNCTION:{
			
			_DumpVector dv;
			
			lua_pushvalue(L, at);
			if(0!=lua_dump(L, vector_writer<_DumpVector>, &dv)){
				lua_error(L);
			}
			lua_pop(L, 1);
			k.add<_DumpVector>(dv, ip, string_types::function); //define function meta type
			if(table2 == interpretation::deep){
				//k.clear_identity();
			}
		}
		break;
		case LUA_TNIL:
			break;
		default:
			_C_ERR("lua type not supported");
			break;				
		};
	}
#endif

}


//extern void register_spaces_replication();
#ifdef _MSC_VER_
extern "C" int __declspec(dllexport) luaopen_spaces(lua_State * L);
#else
extern "C" int luaopen_spaces(lua_State * L);
#endif
//extern int r_initialize_spaceslib(lua_State *L,DSASession* session = NULL);