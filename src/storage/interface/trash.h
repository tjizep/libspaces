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
#if 0



static int spaces_call_environment(lua_State *L) {
	try {
		space_service * pspace = get_space(L, lua_upvalueindex(1));
		umi depth = 0;
		if (lua_isstring(L, 2)) {
			const char * cn = lua_tostring(L, 2);
			spaces_key k;
			to_space_key(L, pspace, k, ipKey(), interpretation::flat, 2);

			for (resolution_iterator ri(k, pspace, pspace->get_path_context()); ri != ri.end(); ++ri) {
				const spaces_key &lk = (*ri);
				if (lk.get_meta_char() == string_types::table) {
					ri.push(L);
					return 1;
				}
				else if (lk.get_meta_char() == string_types::function) {
					if (ri.get_depth()>0) {//neccessary eeviel
						ri.push(L);
						return 1;
					}
				}
				else {auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
					return push_key(L, lk, ipValue());
				}
			}
		}
		// ok now check _G
		i4 t = lua_gettop(L);
		lua_gettable(L, LUA_GLOBALSINDEX);
		t = lua_gettop(L);
	}
	catch (allexceptions&all) {#if 0



static int spaces_call_environment(lua_State *L) {
	try {
		space_service * pspace = get_space(L, lua_upvalueindex(1));
		umi depth = 0;
		if (lua_isstring(L, 2)) {
			const char * cn = lua_tostring(L, 2);
			spaces_key k;
			to_space_key(L, pspace, k, ipKey(), interpretation::flat, 2);

			for (resolution_iterator ri(k, pspace, pspace->get_path_context()); ri != ri.end(); ++ri) {
				const spaces_key &lk = (*ri);
				if (lk.get_meta_char() == string_types::table) {
					ri.push(L);
					return 1;
				}
				else if (lk.get_meta_char() == string_types::function) {
					if (ri.get_depth()>0) {//neccessary eeviel
						ri.push(L);
						return 1;
					}
				}
				else {auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
					return push_key(L, lk, i#if 0



static int spaces_call_environment(lua_State *L) {
	try {
		space_service * pspace = get_space(L, lua_upvalueindex(1));
		umi depth = 0;
		if (lua_isstring(L, 2)) {
			const char * cn = lua_tostring(L, 2);
			spaces_key k;
			to_space_key(L, pspace, k, ipKey(), interpretation::flat, 2);

			for (resolution_iterator ri(k, pspace, pspace->get_path_context()); ri != ri.end(); ++ri) {
				const spaces_key &lk = (*ri);
				if (lk.get_meta_char() == string_types::table) {
					ri.push(L);
					return 1;
				}
				else if (lk.get_meta_char() == string_types::function) {
					if (ri.get_depth()>0) {//neccessary eeviel
						ri.push(L);
						return 1;
					}
				}
				else {auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
					return push_key(L, lk, ipValue());
				}
			}
		}
		// ok now check _G
		i4 t = lua_gettop(L);
		lua_gettable(L, LUA_GLOBALSINDEX);
		t = lua_gettop(L);
	}
	catch (allexceptions&all) {
		get_session_space(L)->reset();
		luaL_error(L, "spaces:%s", all.getdescription().c_str());
	}
	return 1;
}

static int spaces_call(lua_State *L) {
	try {
		space_service * pspace = get_space(L);
		i4 r = 0;
		if (is_space(L, 2)) { //this might be an api call
			string name;
			pspace->get_current().get_name(name);
			if (!pspace->is_api(name)) {

				switch (pspace->get_anapi()) {
				case apis::clone:
					//TODO: pspace->path->context replaced with NULL
					make_space(L, pspace->get_current(), 0);
					return 1;
				case apis::context:
					//TODO: this wont work if paths are switched of
					if (pspace->has_path_context()) {
						make_space(L, pspace->get_path_context()->get_key(), pspace->get_path_context()->context);
					}
					else
						lua_pushnil(L);
					return 1;
				case apis::find:
					make_space_iter(L, pspace)->begin();
					return 1;
				case apis::lower: {
					spaces_key bound;
					to_space_key(L, pspace, bound, ipKey(), interpretation::flat, 3);//only interested in the values not the keys
					make_space_iter(L, pspace)->lower(bound);
				}
								  break;
				case apis::name: {
					string r;
					pspace->get_current().get_name(r);
					lua_pushlstring(L, &r[0], r.size());
					return 1;
				}
								 break;
				default:
					break;
				};

			};//else let it fall through as overridden
		}
		if (pspace->get_current().get_meta_char() == string_types::function) {
			i4 passed = (i4)lua_gettop(L);
			const spaces_uuid_t& identity = pspace->get_current().get_identity();
			lua_pushlstring(L, identity.p(), identity.size());
			lua_gettable(L, LUA_GLOBALSINDEX);
			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				lua_pushlstring(L, identity.p(), identity.size());
				_DumpVector buff;
				pspace->get_current().to_string<_DumpVector>(buff);
				r = luaL_loadbuffer(L, (const i1*)&buff[0], buff.size(), "funct");
				if (r != 0) {
					luaL_error(L, "this space does not contain a function");
				}
				i4 funct = lua_gettop(L);
				lua_newtable(L);// the environment table
				i4 env = lua_gettop(L);
				lua_newtable(L);// the environments meta table
				i4 meta = lua_gettop(L);
				lua_pushvalue(L, 1);// the upvalue for the closure
				lua_pushcclosure(L, spaces_calls->begin(); /// will start a transaction
				lua_setfield(L, meta, "__index"); // leaves the intended meta table	i.e. meta[__index] = function
				lua_setmetatable(L, env); // only the environment table should be left after this
				lua_setfenv(L, funct); // only t6he function is left
				lua_settable(L, LUA_GLOBALSINDEX);
				lua_pushlstring(L, identity.p(), identity.size());
				lua_gettable(L, LUA_GLOBALSINDEX);
			}
			for (i4 a = 2; a <= passed; ++a) {
				lua_pushvalue(L, a);
			}
			lua_call(L, passed - 1, LUA_MULTRET);
			r = lua_gettop(L) - passed;
			return r;
		}
	}
	catch (allexceptions&all) {
		get_session_space(L)->reset();
		luaL_error(L, "spaces:%s", all.getdescription().c_str());
	}
	return 0;
}s
#endif
pValue());
				}
			}
		}
		// ok now check _G
		i4 t = lua_gettop(L);
		lua_gettable(L, LUA_GLOBALSINDEX);
		t = lua_gettop(L);
	}
	catch (allexceptions&all) {
		get_session_space(L)->reset();
		luaL_error(L, "spaces:%s", all.getdescription().c_str());
	}
	return 1;
}

static int spaces_call(lua_State *L) {
	try {
		space_service * pspace = get_space(L);
		i4 r = 0;
		if (is_space(L, 2)) { //this might be an api call
			string name;
			pspace->get_current().get_name(name);
			if (!pspace->is_api(name)) {

				switch (pspace->get_anapi()) {
				case apis::clone:
					//TODO: pspace->path->context replaced with NULL
					make_space(L, pspace->get_current(), 0);
					return 1;
				case apis::context:
					//TODO: this wont work if paths are switched of
					if (pspace->has_path_context()) {
						make_space(L, pspace->get_path_context()->get_key(), pspace->get_path_context()->context);
					}
					else
						lua_pushnil(L);
					return 1;
				case apis::find:
					make_space_iter(L, pspace)->begin();
					return 1;
				case apis::lower: {
					spaces_key bound;
					to_space_key(L, pspace, bound, ipKey(), interpretation::flat, 3);//only interested in the values not the keys
					make_space_iter(L, pspace)->lower(bound);
				}
								  break;
				case apis::name: {
					string r;
					pspace->get_current().get_name(r);
					lua_pushlstring(L, &r[0], r.size());
					return 1;
				}
								 break;
				default:
					break;
				};

			};//else let it fall through as overridden
		}
		if (pspace->get_current().get_meta_char() == string_types::function) {
			i4 passed = (i4)lua_gettop(L);
			const spaces_uuid_t& identity = pspace->get_current().get_identity();
			lua_pushlstring(L, identity.p(), identity.size());
			lua_gettable(L, LUA_GLOBALSINDEX);
			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				lua_pushlstring(L, identity.p(), identity.size());
				_DumpVector buff;
				pspace->get_current().to_string<_DumpVector>(buff);
				r = luaL_loadbuffer(L, (const i1*)&buff[0], buff.size(), "funct");
				if (r != 0) {
					luaL_error(L, "this space does not contain a function");
				}
				i4 funct = lua_gettop(L);
				lua_newtable(L);// the environment table
				i4 env = lua_gettop(L);
				lua_newtable(L);// the environments meta table
				i4 meta = lua_gettop(L);
				lua_pushvalue(L, 1);// the upvalue for the closure
				lua_pushcclosure(L, spaces_calls->begin(); /// will start a transaction
				lua_setfield(L, meta, "__index"); // leaves the intended meta table	i.e. meta[__index] = function
				lua_setmetatable(L, env); // only the environment table should be left after this
				lua_setfenv(L, funct); // only t6he function is left
				lua_settable(L, LUA_GLOBALSINDEX);
				lua_pushlstring(L, identity.p(), identity.size());
				lua_gettable(L, LUA_GLOBALSINDEX);
			}
			for (i4 a = 2; a <= passed; ++a) {
				lua_pushvalue(L, a);
			}
			lua_call(L, passed - 1, LUA_MULTRET);
			r = lua_gettop(L) - passed;
			return r;
		}
	}
	catch (allexceptions&all) {
		get_session_space(L)->reset();
		luaL_error(L, "spaces:%s", all.getdescription().c_str());
	}
	return 0;
}s
#endif

		get_session_space(L)->reset();
		luaL_error(L, "spaces:%s", all.getdescription().c_str());
	}
	return 1;
}

static int spaces_call(lua_State *L) {
	try {
		space_service * pspace = get_space(L);
		i4 r = 0;
		if (is_space(L, 2)) { //this might be an api call
			string name;
			pspace->get_current().get_name(name);
			if (!pspace->is_api(name)) {

				switch (pspace->get_anapi()) {
				case apis::clone:
					//TODO: pspace->path->context replaced with NULL
					make_space(L, pspace->get_current(), 0);
					return 1;
				case apis::context:
					//TODO: this wont work if paths are switched of
					if (pspace->has_path_context()) {
						make_space(L, pspace->get_path_context()->get_key(), pspace->get_path_context()->context);
					}
					else
						lua_pushnil(L);
					return 1;
				case apis::find:
					make_space_iter(L, pspace)->begin();
					return 1;
				case apis::lower: {
					spaces_key bound;
					to_space_key(L, pspace, bound, ipKey(), interpretation::flat, 3);//only interested in the values not the keys
					make_space_iter(L, pspace)->lower(bound);
				}
								  break;
				case apis::name: {
					string r;
					pspace->get_current().get_name(r);
					lua_pushlstring(L, &r[0], r.size());
					return 1;
				}
								 break;
				default:
					break;
				};

			};//else let it fall through as overridden
		}
		if (pspace->get_current().get_meta_char() == string_types::function) {
			i4 passed = (i4)lua_gettop(L);
			const spaces_uuid_t& identity = pspace->get_current().get_identity();
			lua_pushlstring(L, identity.p(), identity.size());
			lua_gettable(L, LUA_GLOBALSINDEX);
			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				lua_pushlstring(L, identity.p(), identity.size());
				_DumpVector buff;
				pspace->get_current().to_string<_DumpVector>(buff);
				r = luaL_loadbuffer(L, (const i1*)&buff[0], buff.size(), "funct");
				if (r != 0) {
					luaL_error(L, "this space does not contain a function");
				}
				i4 funct = lua_gettop(L);
				lua_newtable(L);// the environment table
				i4 env = lua_gettop(L);
				lua_newtable(L);// the environments meta table
				i4 meta = lua_gettop(L);
				lua_pushvalue(L, 1);// the upvalue for the closure
				lua_pushcclosure(L, spaces_calls->begin(); /// will start a transaction
				lua_setfield(L, meta, "__index"); // leaves the intended meta table	i.e. meta[__index] = function
				lua_setmetatable(L, env); // only the environment table should be left after this
				lua_setfenv(L, funct); // only t6he function is left
				lua_settable(L, LUA_GLOBALSINDEX);
				lua_pushlstring(L, identity.p(), identity.size());
				lua_gettable(L, LUA_GLOBALSINDEX);
			}
			for (i4 a = 2; a <= passed; ++a) {
				lua_pushvalue(L, a);
			}
			lua_call(L, passed - 1, LUA_MULTRET);
			r = lua_gettop(L) - passed;
			return r;
		}
	}
	catch (allexceptions&all) {
		get_session_space(L)->reset();
		luaL_error(L, "spaces:%s", all.getdescription().c_str());
	}
	return 0;
}s
#endif
