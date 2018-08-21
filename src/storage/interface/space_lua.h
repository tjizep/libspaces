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
#define SPACES_WEAK_NAME "__spaces_weak__"
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
#if LUA_VERSION_NUM >= 502
        luaL_setfuncs (L, m, 0); /// set functions on meta table (which is in registry)
        lua_pop(L,1l); /// remove remnant
        lua_newtable(L);
        luaL_setfuncs(L, f, 0);
#else
        luaL_register (L, NULL, m);
		luaL_register(L, library_name, f);
#endif
		return 1; ///returns the library
	}

	class top_check{
        lua_State *L;
        int d;
        int t;
    public:
        top_check(lua_State *L,int d):L(L),d(d){
            t = lua_gettop(L);
        }
        ~top_check(){
        	int tt = lua_gettop(L);
            if(tt-t != d){
                luaL_error(L,"invalid stack size expecting %d got %d",d,tt-t);
            }
        }
	};
	
	static size_t lua_tostdstring(lua_State *L,std::string& ss, int at){
		size_t l = 0;
		const char * s  = lua_tolstring(L,at,&l);	
		ss.insert(ss.end(), s,s+l);
		return l;
	}
	static void * is_udata(lua_State *L, int ud, const char* tname) {
		//return lua_touserdata(L, ud);
		return luaL_checkudata(L, ud, tname);
	}
	template<typename _TypeName>
	static _TypeName& err_check_ref_udata(lua_State *L, const char* tname, int ud = 1) {
		void *p = is_udata(L, ud, tname);
		if (p != NULL) {  /* value is a userdata? */
			return *(_TypeName*)p;
		}

		luaL_error(L, "Type %s expected",tname);  /* else error */

		throw std::exception("invalid lua type");
	}
	template<typename _TypeName>
	static _TypeName* err_checkudata(lua_State *L, const char* tname, int ud = 1) {
		void *p = is_udata(L, ud, tname);
		if (p != NULL) {  /* value is a userdata? */
			return (_TypeName*)p;
		}
		luaL_error(L, "Type %s expected",tname);  /* else error */

		return nullptr;  /* to avoid warnings */
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
	template<typename _sT, typename _pT, typename _p1T >
	static _sT* create_instance_from_nothing(lua_State *L, _pT param, _p1T param1) {
		size_t nbytes = sizeof(_sT);
		_sT *k = (_sT *)lua_newuserdata(L, nbytes);
		new (k) _sT(param,param1);
		return k;  /* new userdatum is already on the stack */

	}
	template<typename _sT, typename _pT, typename _p1T >
	static std::shared_ptr<_sT> create_shared_instance_from_nothing(lua_State *L, _pT param, _p1T param1) {
		size_t nbytes = sizeof(std::shared_ptr<_sT>);
		std::shared_ptr<_sT> *k = (std::shared_ptr<_sT> *)lua_newuserdata(L, nbytes);
		new (k) std::shared_ptr<_sT>();
		*k = std::make_shared<_sT>(param,param1);
		return *k;  /* new userdatum is already on the stack */

	}
	template<typename _sT>
	static void assign_shared_instance_from_nothing(lua_State *L, std::shared_ptr<_sT> other) {
		size_t nbytes = sizeof(std::shared_ptr<_sT>);
		std::shared_ptr<_sT> *k = (std::shared_ptr<_sT> *)lua_newuserdata(L, nbytes);
		new (k) std::shared_ptr<_sT>(other);
	}
	template<typename _SessionType>
	static void push_session(lua_State *L, const char *name, typename _SessionType::ptr other, bool is_reader) {

		assign_shared_instance_from_nothing<_SessionType>(L,other);
		other->set_state(L);
		// set its metatable
		luaL_getmetatable(L, SPACES_SESSION_LUA_TYPE_NAME);
		if (lua_isnil(L, -1)) {
			luaL_error(L, "no meta table of type %s", SPACES_SESSION_LUA_TYPE_NAME);

		}else{
			lua_setmetatable(L, -2);
			other->check();
			other->set_mode(is_reader);
		}

	}

	template<typename _SessionType>
	static typename _SessionType::ptr& get_session(lua_State *L, int at = 1) {
		typename _SessionType::ptr *r = nullptr;
		r = err_checkudata<typename _SessionType::ptr>(L, SPACES_SESSION_LUA_TYPE_NAME, at);

		(*r)->check();
		return *r;
	}
	#define tofilep(L)	((FILE **)luaL_checkudata(L, 1, LUA_FILEHANDLE))



    template<typename _SessionType>
    class lua_session : public spaces_session<_SessionType>{
	public:
		typedef lua_session<_SessionType> session_type;

		typedef typename session_type::space space;
		typedef std::vector<i1> _DumpVector;
		typedef rabbit::unordered_map<ptrdiff_t, space*> _LuaKeyMap;
		typedef std::shared_ptr<lua_session> ptr;

	private:
		lua_State *L;
		/// despite improvements
		/// sometimes the type system still gets lost without
		/// a hint
		typedef session_type _Self;
	public:

		typedef typename spaces_session<_SessionType>::_Set _Set;
		struct lua_iterator : public spaces::spaces_iterator<_Set>{
			typedef typename spaces::spaces_iterator<_Set> super;
			lua_iterator * sentinel;
			lua_iterator() :session(nullptr){
				this->sentinel = this;
			}
			~lua_iterator(){
				if(this->sentinel != this){
					err_print("destroying class with invalid sentinel");
				}
				this->sentinel = nullptr;
			}
			_Set& get_set(){
			    return session->get_set();
			}
			void recover(){

				if(this->db != session->get_dbms()){
					this->db = session->get_dbms();
					super::recover(get_set());

					return;
				}

			   if(!this->is_valid())
					super::recover(get_set());
			}
			void set_session(ptr session){
				this->session = session;
				this->db = session->get_dbms();
			}
			bool has_session() const {
				return session!=nullptr;
			}
			ptr& get_session(){
				return session;
			}
		private:
			spaces::dbms::ptr db;
			ptr session;

		};
		lua_session(const std::string& name, bool reader) : L(nullptr),spaces_session<_SessionType>(name,reader) {
			dbg_print("create session:%s",name.c_str());
		}
		~lua_session() {
			dbg_print("closing session '%s'",get_name().c_str());

		}

		void set_state(lua_State *L) {
			this->L = L;
		}
		const lua_State* get_state() const {
			return this->L;
		}
        bool is_integer(double n) {
			return (n - (i8)n == 0);
		}
		lua_iterator* create_iterator(ptr session) {
			auto i = create_instance_from_nothing<lua_iterator>(L); /* new userdatum is already on the stack */
			i->set_session(session);
			return i;
		}
		lua_iterator*  get_iterator(int at = 1) {
			lua_iterator * i = err_checkudata<lua_iterator>(L, SPACES_ITERATOR_LUA_TYPE_NAME, at);
			//i->session = this;
			return i;

		}
		space* create_space_from_nothing() {
			return create_instance_from_nothing<space>(L);  /* new userdatum is already on the stack */

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

		space * is_space(_LuaKeyMap& keys, int at = 1) const {
			if (lua_istable(L, at)) {
				ptrdiff_t pt = reinterpret_cast<ptrdiff_t>(lua_topointer(L, at));
				auto f = keys.find(pt);
				if(f!=keys.end()){
					return f->second;
				}
			}
			return nullptr;
		}
		space * get_space(_LuaKeyMap& keys, int at = 1) {
			space * space = is_space(keys,at);
			if(space==nullptr){
				luaL_error(L, "no key associated with table of type %s", SPACES_LUA_TYPE_NAME);
			}
			return space;
		}
		void to_space
				(	_LuaKeyMap& keys
					,	spaces::key& d
					,   spaces::record& r
					, 	int _at = 2
				) {
			int at = _at;
			if (at < 0)
				at = lua_gettop(L) + (1 + at);//translate to absolute position
			int lt = lua_type(L, at);
			switch (lt) {
				case LUA_TTABLE: {
					auto space = is_space(keys,at);
					if(space!=nullptr){
						if (this->link(r,space)) break;
					}
					this->resolve_id(d,r,true); /// assign an identity to p (its leaves will need it)
					lua_pushnil(L);
					/// will push the name and value of the current item (as returned by closure)
					/// on the stack
					while (lua_next(L, at) != 0) {
						typename _Self::space s;
						s.first.set_context(r.get_identity()); //
						to_space_data(s.first.get_name(), -2);	// just set the key part
						to_space(keys,s.first, s.second, -1); // d becomes a parent and will receive an id if -1 is another table
						/// add new key to table
						this->insert_or_replace(s);
						lua_pop(L, 1);
					}

				}break;
				case LUA_TUSERDATA: {
					/// add the link here
					space * l = err_checkudata<space>(L, SPACES_LUA_TYPE_NAME, at);
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
		void create_weak_globals(const std::string& _storage){
			const char * storage = _storage.c_str();
			lua_getglobal(L,storage);
			if(lua_isnil(L,-1)){
				int ts = lua_gettop(L);
				lua_pop(L,1);
				lua_newtable(L); // new_table={}
				lua_newtable(L); // metatable={}
				lua_pushliteral(L, "__mode");
				lua_pushliteral(L, "kv");
				lua_rawset(L, -3); // metatable.__mode='kv'
				lua_setmetatable(L, -2); // setmetatable(new_table,metatable)
				lua_setglobal(L,storage);
				lua_getglobal(L,storage);

			}

		}
		space* open_space(_LuaKeyMap& keys,ptr session, ui8 id) {
			i4 t = lua_gettop(L);
			if (!is_space(keys, -1)) {
				dbg_print("open space: %lld on storage %s",(nst::lld)id,session->get_name().c_str());

				create_weak_globals(session->get_name());
				i4 tg = lua_gettop(L);
				lua_pushnumber(L,id);
				lua_gettable(L,tg);
				i4 t1 = lua_gettop(L);
				space* r = nullptr;
				if(!lua_isnil(L,-1)){
					lua_remove(L,tg);
					r = is_space(keys,-1);
					if(r != nullptr){
						t1 = lua_gettop(L);
						if(t1 - t != 1){
							luaL_error(L,"invalid stack size");
						}
						dbg_print("open space: using cached %lld on storage %s",(nst::lld)id,r->get_session()->get_name().c_str());
						if(r->get_session()->get_name() != session->get_name()){
							luaL_error(L,"open space: invalid storage");
						}
						return r;
					}else{
						luaL_error(L,"not a space");
					}
				}
				lua_pop(L,1);

				lua_newtable(L);

				lua_pushnumber(L,id);
				lua_pushvalue(L,-2);
				lua_settable(L,tg);

				r = new space(); // spaces::create_key_from_nothing(L);
				// set its metatable
				luaL_getmetatable(L, SPACES_LUA_TYPE_NAME);
				if (lua_isnil(L, -1)) {
					luaL_error(L, "no meta table of type %s", SPACES_LUA_TYPE_NAME);
				}
				lua_setmetatable(L, -2);
				ptrdiff_t pt = reinterpret_cast<ptrdiff_t>(lua_topointer(L, -1));
				keys[pt] = r;
				r->set_session(session);
				lua_remove(L,tg);
				if(is_space(keys,-1)==nullptr){
					luaL_error(L,"not a space");
				}
				t1 = lua_gettop(L);
				if(t1 - t != 1){
					luaL_error(L,"invalid stack size");
				}
				return r;
			}
			else {
				space* r = this->get_space(keys,-1);
				r->set_session(session);
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
				dbg_print("set number on space [%.3f]",n);
				d = n;
			}break;
			case LUA_TSTRING: {
				size_t l = 0;
				const char * v = ::lua_tolstring(L, at, &l);
				dbg_print("set string on space [%s]",v);
				d.set_text(v, l);
			}break;
			case LUA_TBOOLEAN: {
				bool b = (lua_toboolean(L, at) != 0);
				dbg_print("set boolean on space [%s]",b ? "true" : "false");
				d = b;
			}break;
			case LUA_TTABLE: {
				dbg_print("could not set table as space");
				/// TODO: multi data format
				luaL_error(L,"could not set table as key");
			}break;
			case LUA_TUSERDATA: {
				dbg_print("could not set user data on space");
				/// TODO: multi data format

			}break;
			case LUA_TFUNCTION: {
				dbg_print("compile lua bytecode function into space");
				typename _Self::_DumpVector dv;

				lua_pushvalue(L, at);
				if (0 != lua_dump(L, vector_writer<typename _Self::_DumpVector>, &dv)) {
					lua_error(L);
				}
				lua_pop(L, 1);
				d.set_function(dv);

			}break;
			case LUA_TNIL:
			{	dbg_print("could not assign nil to space");
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
				dbg_print("spaces data: push number %.3f",d.to_number());
				lua_pushnumber(L, d.to_number());
				break;
			case data_type::boolean:
				dbg_print("spaces data: push boolean %lld",(nst::lld)d.to_number());
				lua_pushboolean(L, (int)d.get_integer());
				break;
			case data_type::text:
				dbg_print("spaces data: push text %s",d.get_sequence().data());
				lua_pushlstring(L, d.get_sequence().data(), d.get_sequence().size());
				break;
			case data_type::function:
				dbg_print("spaces data: push function ");
				lua_pushnil(L);
				break;
			case data_type::infinity:
				dbg_print("spaces data: push infinity ");
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
		(	_LuaKeyMap& keys
		,	space& d
		, 	int _at = 2
		) {
			to_space(keys, d.first,d.second,_at);
		}
		int push_pair(_LuaKeyMap& keys,ptr session, const spaces::key& k,const spaces::record& v) {
			int r = push_data(k.get_name());
			if (v.get_identity() != 0) {
				space * s = this->open_space(keys,session,v.get_identity());
				s->first = k;
				s->second = v;
				++r;
			}
			else {
				r += push_data(v.get_value());
			}
			return r;
		}
		const std::string& get_name() const{
			return this->session.get_name();
		}
		int push_space(const space& s) {
			return push_pair(s.first, s.second);
		}
	}; /// lua_session
	typedef lua_session<db_session> session_t;
	typedef session_t::space space;


}


//extern void register_spaces_replication();
#ifdef _MSC_VER
extern "C" int __declspec(dllexport) luaopen_spaces(lua_State * L);
extern "C" int __declspec(dllexport) luaclose_spaces(lua_State * L);
#else
extern "C" int luaopen_spaces(lua_State * L);
extern "C" int luaclose_spaces(lua_State * L);
#endif
//extern int r_initialize_spaceslib(lua_State *L,DSASession* session = NULL);