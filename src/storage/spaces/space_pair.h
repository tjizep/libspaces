#pragma once
#include <platform/librio/stlin.h>
#include <platform/librio/err.h>
#include <platform/usage.h>
#include <storage/spaces/data_type.h>


namespace spaces{
	typedef ui1 _TypeB;
	typedef ui4 _TypeL;
	static const _TypeB TYPE_MASK = 15;
	static const _TypeB SIZE_MASK = 240;
	enum
	{	F_CONTEXT_FROM_REF = 1
	,	F_CONTEXT_NIL = 2
	,	F_KEY_SIZE_8 = 4
	,	F_ID_NIL = 8
	,	F_VALUE_SIZE_8 = 16
	,	F_VALUE_EMPTY = 32
	,	F_CONTAINER = 64
	};
#ifndef _DEBUG
#ifdef _USE_NED_
	typedef nedalloc::nedallocator<ui1, nedalloc::nedpolicy::reserveN<16>::policy > _Allocator;
#else
	typedef std::allocator<ui1> _Allocator;
#endif
#else
	typedef std::allocator<ui1> _Allocator;
#endif
	inline umi write_size(_TypeL size, ui1* tot){
		if(size < 14){
			tot[0] |= (((ui1)size) << 4) ;
			return 0;
		}else if(size < 256){
			tot[0] |= ( 14 << 4 ) ;
			tot[1] = (ui1)size;
			return 1;
		}else{
			tot[0] |=  ( 15 << 4 ) ;
			*((_TypeL*)(tot+1)) = (_TypeL)size;
			return sizeof(_TypeL);
		}
	}
	inline _TypeL read_size(const ui1* tot){
		ui1 m = ((*tot) & SIZE_MASK) >> 4;
		switch(m){
		case 14:
			return *(tot+1);
			break;
		case 15:
			return *((_TypeL*)(tot+1));
			break;
		}
		return m;
	}
	inline _TypeL read_size_len(const ui1* tot)  {
		ui1 m = ((*tot) & SIZE_MASK) >> 4;
		switch(m){
		case 14:
			return *(tot+1) + 1;
			break;
		case 15:
			return *((_TypeL*)(tot+1))+sizeof(_TypeL);
			break;
		}
		return m;
	}
	inline umi size_len(const ui1* tot){
		ui1 m = ((*tot) & SIZE_MASK) >> 4;
		switch(m){
		case 14:
			return 1;
			break;
		case 15:
			return sizeof(_TypeL);
			break;
		}
		return 0;
	}
	inline ui4 size_len(const _TypeL size){
		if(size > 255){
			return sizeof(_TypeL);
		}else if(size > 13){
			return 1;
		}else {
			return 0;
		}
	}
	class _StaticSpacePair{
	protected:
		const ui1 *_data;

	public:

		_StaticSpacePair()
		:	_data(0)
		{
		}
		_StaticSpacePair(const ui1* record){
			assign(record);
		}
		void assign(const ui1* record){
			_data = record;
		}
		inline ui4 rec_size(){
			return read_size_len(_data) + sizeof(_TypeB);
		}
		void next(){
			_data += rec_size();
		}
		_StaticSpacePair(const _StaticSpacePair& right){
			*this = right;
		}
		_StaticSpacePair& operator=(const _StaticSpacePair& right){
			_data = right._data;
			return *this;
		}
		template<typename _tC>
		const _tC &cast() const {
			return *(_tC*)&data()[0];
		}
		template<typename _tC>
		_tC &cast() {
			return *(_tC*)&data()[0];
		}
		inline const _TypeB t() const{
			return TYPE_MASK & (*((const _TypeB*)_data));
		}
		inline const ui1* data() const{
			//return assigned;
			return _data+sizeof(_TypeB)+size_len(_data);

		}

		inline const ui1* sdata() const{
			return _data+sizeof(_TypeB)+size_len(_data);

		}
		const ui4 size() const{
			return read_size(_data);
		}
		const _TypeB* operator*() const{
			return data();
		}
	};
	inline ui4 size_off(const ui1* data, ui1 flags){
		if((flags & F_VALUE_SIZE_8) == 0){
			return NTOHL(*(ui4*)(data+1));
		}else{
			return (ui1)*(data+1);
		}
	}
	class _SimpleStaticPair{
	protected:
		ui1 flags;
		_TypeB type;
		ui4 _size;
		const ui1 *_data;

	public:
		ui4 size() const {
			return _size;
		}
		_SimpleStaticPair()
		:	_data(0)
		,	_size(0)
		{
		}
		_SimpleStaticPair(const ui1* record,ui1 flags):flags(flags){
			assign(record,flags);
		}
		void assign(const ui1* record,ui1 flags){
			_data = record;
			type = *(_TypeB*)_data;
			_data += sizeof(_TypeB);
			if((flags & F_VALUE_SIZE_8) == 0){
				_size = NTOHL(*(ui4*)_data);
				_data += sizeof(ui4);
			}else{
				_size = (ui1)*_data;
				++_data;
			}
		}

		_SimpleStaticPair(const _SimpleStaticPair& right){
			*this = right;
		}
		_SimpleStaticPair& operator=(const _SimpleStaticPair& right){
			_data = right._data;
			return *this;
		}
		template<typename _tC>
		const _tC &cast() const {
			return *(_tC*)&data()[0];
		}
		template<typename _tC>
		_tC &cast() {
			return *(_tC*)&data()[0];
		}
		inline const _TypeB t() const{
			return type;
		}
		inline const ui1* data() const{
			return _data;
		}

		inline const ui1* sdata() const{
			return _data;

		}
		//TODO: this could be added
		/*const ui4 size() const{
			return read_size(_data);
		}*/
		const _TypeB* operator*() const{
			return data();
		}
	};
	//extern _Allocator pair_alloc;
	template<int _DATA_SIZE,ui4 _MAX_VALUE_ = 52200>
	class _SpacePair{
	protected:
		static const int USE = 0;
		//static const int _DATA_SIZE = 123;
		ui1 _data[_DATA_SIZE];
		_TypeB _type;
		ui4 _size;
		ui4 _capacity;
		inline void short_clear(){
			//if(_size <= _DATA_SIZE)
			//	*(ui8*)&data()[1] = 0ull;
		}
		inline _Allocator get_allocator(){

			return _Allocator();
		}
	public:
		inline _TypeB& t(){
			return _type;
		}
		inline const _TypeB t() const{
			return ( TYPE_MASK & _type );
		}
		ui4 serialized() const {
			return sizeof(_TypeB) + size_len(_size) + _size;
		}
		void serialize(ui1* data) const {
			*(_TypeB*)data = _type;
			umi sl = write_size(_size,data);
			const ui1 * d = this->data();
			memcpy(data+sizeof(_TypeB)+sl, d, _size);
		}
		_SpacePair(){
			_capacity = _DATA_SIZE;
			_size = 0;
			short_clear();
			t() = final;
			//assigned = &_data[0];

		}
		~_SpacePair(){
			if(_capacity > _DATA_SIZE){

				//free(data());
				usage.remove(USE,_capacity);
				get_allocator().deallocate(data(), _capacity);
			}
		}
		ui1* resize_empty(ui4 ns){
			if(ns > _MAX_VALUE_){
				_C_ERR("invalid value size");
			}
			resize(ns);
			return data();
		}
		inline size_t round_up(size_t s) const {
			return ((s/8)+1)*8;
		}
		void resize(ui4 ns){
			if(ns > _MAX_VALUE_){
				_C_ERR("invalid value size");
			}
			if(ns > _capacity){
				ui1 * n_data = get_allocator().allocate(round_up(ns));
				memcpy(n_data,data(),min(Cast::c(ns), size()));
				if(_capacity > _DATA_SIZE){
					usage.remove(USE, _capacity);
					get_allocator().deallocate(data(),_capacity);
				}
				_capacity = Cast::c(round_up(ns));
				usage.add(USE, _capacity);
				memcpy(_data, &n_data, sizeof(ui1*));
			}
			_size = ns;
		}
		inline bool empty() const {
			return (_size==0);
		}
		void clear(){
			_size = 0;
			t() = final;
		}
		_SpacePair(const _SpacePair& right) {
			_capacity = _DATA_SIZE;
			t() = final;
			*(ui8*)_data = 0;
			_size = 0;
			*this = right;
		}
		_SpacePair& operator=(const _SpacePair& right){
			t() = right.t();

			resize(right.size());
			if(_size <= _DATA_SIZE)
				short_clear();
			memcpy(data(), right.data(), right.size());
			return *this;
		}
		template<typename _tC>
		_tC &cast() const {
			return *(_tC*)&data()[0];
		}
		inline ui1* data() {
			//return assigned;
			if(_capacity > _DATA_SIZE){
				return (ui1*)(*(smi*)(_data));
			}else
				return &_data[0];

		}
		inline const ui1* data() const{
			//return assigned;
			if(_capacity > _DATA_SIZE){
				return (ui1*)(*(smi*)(_data));
			}else
				return &_data[0];

		}
		inline const ui1* sdata() const{
			return &data()[0];

		}
		inline ui4 capacity() const {
			return _capacity; //sizeof(_data);

		}
		inline ui4 allocated() const {
			if(_capacity > _DATA_SIZE){
				return _capacity + sizeof(*this);
			}else
				return sizeof(*this);

		}
		const ui4 size() const{
			return _size;
		}
		const ui1* operator*() const{
			return data();
		}
		template<typename _Primitive>
		void add_primitive(const _Primitive& indata){
			ui4 at = size();
			resize(at + sizeof(_Primitive));
			*((_Primitive*)&data()[at])=indata;
			//_size+=sizeof(_Primitive);
		}
		template<typename _PairType>
		void set_pair(const _PairType &_l){
			const ui1 * d = _l.sdata();
			i4 l = _l.size();
			resize(l);
			::memcpy(&(data()[0]),d,l);
			t() = _l.t();
		}
		template<typename _ValueType>
		void add_buffer(const _ValueType* data1, umi l){
			//_C_ASSERT(l*sizeof(_ValueType) >= 1<<16,"invalid buffer length");
			i4 vl = l*sizeof(_ValueType) ;

			ui4 at = size();
			t() = (vl+at) < 256 ? char_sequence8:char_sequence;
			resize(at+vl);
			::memcpy(&(data()[at]),data1,vl);
		}
		template<typename _ValueType>
		void add_buffers(const _ValueType* data1, umi l1, const _ValueType* data2, umi l2){
			//_C_ASSERT((l1*sizeof(_ValueType) >= 1<<16)||((l2*sizeof(_ValueType)) >= 1<<16),"invalid buffer length");
			i4 vl = (l1+l2)*sizeof(_ValueType);

			ui4 at = size();
			t() = (at+vl) < 256 ? char_sequence8:char_sequence;
			if(at==0) short_clear();
			resize(vl+at);
			::memcpy(&(data()[at]),data1,l1*sizeof(_ValueType));
			at += l1*sizeof(_ValueType);
			::memcpy(&(data()[at]),data2,l2*sizeof(_ValueType));
		}
		template<typename _Primitive, typename _ValueType>
		void add_primitive_and_buffer(const _Primitive data1, const _ValueType* data2, umi l2){
			//_C_ASSERT((l1*sizeof(_ValueType) >= 1<<16)||((l2*sizeof(_ValueType)) >= 1<<16),"invalid buffer length");
			register i4 vl = l2*sizeof(_ValueType) + sizeof(data1);
			register ui4 at = size();
			t() = (at+vl) < 256 ? char_sequence8:char_sequence;

			if(at==0) short_clear();
			resize(vl+at);
			*((_Primitive*)&data()[at])=data1;
			at += sizeof(data1);
			::memcpy(&(data()[at]),data2,l2*sizeof(_ValueType));
		}
		template<typename _Primitive, typename _ValueType>
		void set_primitive_and_buffer(const _Primitive data1, const _ValueType* data2, umi l2){
			//_C_ASSERT((l1*sizeof(_ValueType) >= 1<<16)||((l2*sizeof(_ValueType)) >= 1<<16),"invalid buffer length");
			register i4 vl = l2*sizeof(_ValueType) + sizeof(data1);
			t() = vl < 256 ? char_sequence8:char_sequence;
			register ui4 at = 0;
			if(at==0) short_clear();
			resize(vl);
			*((_Primitive*)&data()[at])=data1;
			at += sizeof(data1);
			::memcpy(&(data()[at]),data2,l2*sizeof(_ValueType));
		}
		template<typename _Vector>
		void add_vector(const _Vector& data){
			add_buffer<_Vector::value_type>(&data[0],data.size());
		}
		template<typename _Vector>
		void add_vectors(const _Vector& data1, const _Vector& data2){
			add_buffers<_Vector::value_type>(&data1[0],data1.size(),&data2[0],data2.size());
		}
	};
};
