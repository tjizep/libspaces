#pragma once
#include <string>
#include <algorithm>
#include <iterator>
#include <limits>
#include <storage/spaces/data_type.h>
#include <stx/storage/basic_storage.h>
#include <rabbit/unordered_map>

#ifdef _MSC_VER
#define SPACES_NOINLINE_PRE _declspec(noinline)
#define SPACES_NOINLINE_
#else
#define SPACES_NOINLINE_PRE
#define SPACES_NOINLINE_ __attribute__((noinline))
#endif
namespace spaces {
	namespace nst = stx::storage;
	struct data_type {
		
		enum {
			none,
			numeric,
			boolean,
			text, 
			function,
			multi,
			infinity
		};
	};
	class astring {
	public:
		typedef typename std::vector<char, stx::storage::allocation::pool_alloc_tracker<char>> tracked_buffer;
	private:
		static const i4 SS = 256;
		ui4 l;
		i1 sequence[sizeof(tracked_buffer)];
		tracked_buffer & make_long() {
			new (sequence) tracked_buffer();
			l = SS;
			return (tracked_buffer&)sequence;
		}

		tracked_buffer&str() {
			return (tracked_buffer&)sequence;
		}
		const tracked_buffer &str() const {
			return (tracked_buffer&)sequence;
		}
		void resize(ui4 l, const char * data) {
			if (l >= sizeof(sequence)) {
				tracked_buffer &s = make_long();
				s.resize(l);
				memcpy(&s[0], data, l);
				return;
			}
			this->l = l;
			memcpy(sequence, data, l);

		}
		SPACES_NOINLINE_PRE /// assume the comparison of long strings will happen less often 
		i4 compare_data_long(const astring& right) const
		SPACES_NOINLINE_ /// other compilers
		{
			return memcmp(data(), right.data(), std::min<i4>(size(), right.size()));
		}
		// FNV-1a hash function for bytes
		// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
		// https://tools.ietf.org/html/draft-eastlake-fnv-13#section-6
		// used to attempt to fix bad hashes from code
		size_t fnv_1a_bytes(const unsigned char *bytes, size_t count) const {
			const unsigned long long FNV64prime = 0x00000100000001B3ull;
			const unsigned long long FNV64basis = 0xCBF29CE484222325ull;
			size_t r = FNV64basis;
			for (size_t a = 0; a < count; ++a){
				r ^= (size_t)bytes[a]; // folding of one byte at a time
				r *= FNV64prime;
			}
			return r;
		}
	public:
		MSGPACK_DEFINE_ARRAY(l,sequence)
		size_t hash() const{
			return fnv_1a_bytes(decoded(),size());
		}
		void clear() {
			if (is_long()) {
				str().clear();
			}
			else {
				l = 0;
			}
		}
		bool is_long() const {
			return l == SS;
		}
		void resize(ui4 l) {
			if (l >= sizeof(sequence)) {
				make_long().resize(l);
				return;
			}
			this->l = l;
		}		
		void set_data(const char * data,ui4 l) {
			resize(l,data);			
		}
		template<typename _tT>
        void set_data(const std::vector<_tT>& data) {
            resize(data.size(),(const char*)data.data());
        }
		ui4 size() const {
			return l == SS ? (ui4)(str().size()) :
				l;
		}
		const char *data() const {
			return l == SS ? str().data() :
				sequence;
		}
		const char *c_str() const {
			return l == SS ? str().data() :
				sequence;
		}
		char *writable(){
			return l == SS ? &str()[0] :
				sequence;
		}
		const unsigned char *decoded() const {
			return (const unsigned char *)data();
		}
		const char *readable() const {
			return l == SS ? &str()[0] :
				   sequence;
		}
		
		i4 compare(const astring& right) const {
			i4 r = 0;
			if (right.l != SS && l != SS) {
				r = memcmp(sequence, right.sequence, std::min<i4>(l, right.l));
			}
			else {
				r = compare_data_long(right);
			}
			if (r == 0) {
				return l - right.l;
			}
			return r;		
		}
		astring() : l(0){
            *((nst::u64*)sequence) = 0ull;

		}
		astring(const astring& right) : l(0) {
			*this = right;
		}
		~astring() {
			if (is_long()) {
			    typedef std::vector<char> vchar;
				str().~tracked_buffer();
			}
		}
		astring& operator=(const astring& right) {
			if (!right.is_long()) {
				set_data(right.sequence, right.l);
			}
			else {
				set_data(right.data(), right.size());
			}
			return *this;
		}
		astring& operator=(const std::string& right) {
			
			set_data(right.data(), (ui4)right.size());
			return *this;
		}

	};
	class data {
	private:
		i4 type;
		astring sequence;

	public:
		MSGPACK_DEFINE_ARRAY(type,sequence)
		data(const data&r)
		: 	sequence(r.sequence)
		, 	type(r.type){

		}
		void clear() {
			type = data_type::numeric;
			sequence.clear();
		}
		data() {
			type = data_type::numeric;

			
		}
		template<typename T>
		T& cast_sequence(){
            sequence.resize(sizeof(T));

            return *(T*)(sequence.writable());
		}
		template<typename T>
		const T& cast_sequence() const {
			return *(T*)(sequence.readable());
		}

		double& get_double(){
			return cast_sequence<double>();
		}
		double& get_number(){
			return cast_sequence<double>();
		}
		i8& get_integer(){
			return cast_sequence<i8>();
		}
		const double& get_double() const {
			return cast_sequence<double>();
		}
		const double& get_number() const{
			return cast_sequence<double>();;
		}
		const i8& get_integer() const{
			return cast_sequence<i8>();
		}
		data& operator=(const data& r) {
			type = r.type;
			sequence = r.sequence;
			return *this;
		}
		data& operator=(const ui8& r) {
			clear();
			type = data_type::numeric;
			get_integer() = r;
			return *this;
		}
		data& operator=(const bool& r) {
			clear();
			type = data_type::boolean;
			get_integer() = r;
			return *this;
		}
		data& operator=(const double& r) {
			clear();
			type = data_type::numeric;
			get_number() = r;
			double t = get_number();
			if(t != r){
				err_print("assignment failed");
			}
			return *this;
		}
		data& operator=(const std::string& r) {
			clear();
			type = data_type::text;			
			sequence = r;
			return *this;
		}
		void make_infinity() {
			clear();
			type = data_type::infinity;
			get_number() = std::numeric_limits<f8>::infinity();
		}
		void set_text(const i1* s, const size_t l) {
			clear();
			type = data_type::text;
			sequence.set_data(s, (i4)l);
			
		}
		void set_string(const std::string& s) {
			clear();
			type = data_type::text;
			sequence.set_data(s.data(), (ui4)s.size());

		}
		template<typename _VectorType>
		void set_function(const _VectorType& vt) {
			clear();
			type = data_type::function;
			sequence.set_data<typename _VectorType::value_type>(vt);

		}
		i4 compare(const data&right) const {
			if (type != right.type) return type - right.type;
			if (is_text()) {				
				return sequence.compare(right.sequence);
			}else {			
				if(get_number() < right.get_number()) return -1;
				if(get_number() > right.get_number()) return 1;
			}
			return 0;
		}
		bool operator != (const data&r) const {
			return compare(r) != 0;
		}
		bool operator < (const data& r) const {
			return compare(r) < 0;
			
		}

		astring& get_sequence(){
			return sequence;
		}
		const astring& get_sequence() const {
			return sequence;
		}
		bool is_text() const {
			return type == data_type::text;
		}
		f8 to_number() const {
			char* end;
			switch (type) {
			case data_type::numeric:
				return get_number();
			case data_type::boolean:
				return get_number();
			case data_type::text:
				return std::strtod(this->sequence.c_str(), &end);
			case data_type::function:				
			case data_type::multi:
			case data_type::infinity:
			default:
				break;
			}
			return std::numeric_limits<f8>::quiet_NaN();
		}

		ui4 size() const {
			char* end;
			switch (type) {
				case data_type::numeric:
					return 8;
				case data_type::boolean:
					return 8;
				case data_type::text:
					return this->sequence.size();
				case data_type::function:
				case data_type::multi:
				case data_type::infinity:
				default:
					break;
			}
			return 8;
		}
		i4 get_type() const {
			return this->type;
		}
		nst::u32 stored() const {
			nst::i32 ts = sizeof(ui8);
			nst::i32 ss = sequence.size() + sizeof(sequence.size());
			nst::i32 ns = sizeof(ui8);
			return ts + (this->is_text() ? ss : ns);
		};
		
		nst::buffer_type::iterator store(nst::buffer_type::iterator w) const {
			nst::buffer_type::iterator writer = nst::primitive::store(w, (ui8)this->type);
			if (this->is_text()) {
				writer = nst::primitive::store(writer, sequence.size());
				memcpy((nst::u8*)&(*writer), sequence.data(), sequence.size());
				writer += sequence.size();
			}
			else {
				writer = nst::primitive::store(writer, get_integer());
			}
			if (writer - w != stored()) {
				ptrdiff_t diff = writer - w;
				ptrdiff_t stred = stored();
				err_print("wrote wrong byte count");
			}
			return writer;
		};

		nst::buffer_type::const_iterator read(const nst::buffer_type& buffer, nst::buffer_type::const_iterator r) {
			
			nst::buffer_type::const_iterator reader = r;
			ptrdiff_t diff = reader - r;
			if (reader != buffer.end()) {
				ui8 t = 0;
				reader = nst::primitive::read(t, reader);
				this->type = t;
				if (is_text()) {

					auto s = sequence.size();
					reader = nst::primitive::read(s, reader);
					sequence.resize(s);
					memcpy(sequence.writable(), (nst::u8*)&(*reader), sequence.size());
					reader += sequence.size();
				}
				else {
					diff = reader - r;
					reader = nst::primitive::read(this->get_integer(), reader);
					diff = reader - r;
				}
			}
			if (reader - r != stored()) {
				diff = reader - r;
				ptrdiff_t stred = stored();
				err_print("wrote wrong byte count");
			}
			return reader;
		};

		size_t hash() const {
			char* end;
			switch (type) {
				case data_type::numeric:
				case data_type::boolean:
					return get_integer();
				case data_type::text:
					return this->sequence.hash();
				//case data_type::function:
				//case data_type::multi:
				//case data_type::infinity:
				default:
					break;
			}
			return 0;

		}
	};
	class key {
	private:
		ui8 context;
		data name;
	
	public:
		MSGPACK_DEFINE_ARRAY(context,name)
		static const bool use_encoding = true;
		key(const key&r) : context(r.context), name(r.name){

		}
		key() {
			context = 0;
		}
		key& operator=(const key& r) {
			name = r.name;
			context = r.context;			
			return *this;
		}

		bool operator != (const key&r) const {
			if (context != r.context) return true;
			if (name != r.name) return true;
			return false;
		}
		bool operator == (const key&r) const {
			return !(*this != r);
		}
		bool operator < (const key&r) const {
			if (context != r.context) return context < r.context;			
			int l = name.compare(r.name);
			if (l < 0) return true;
			return false;
		}
		bool found (const key&r) const {
			if (context != r.context) return false;
			if (name != r.name) return false;
			return true;
		}
		ui8 get_context() const {
			return this->context;
		}
		void set_context(ui8 context) {
			this->context = context;
		}
		data& get_name() {
			return name;
		}
		void set_name(const data& name){
			this->name = name;
		}
		const data& get_name() const{
			return name;
		}

	public:
		/// persistence functions		
		nst::buffer_type::const_iterator read(const nst::buffer_type& buffer, typename nst::buffer_type::const_iterator& r) {
			nst::buffer_type::const_iterator reader = r;

			if (reader != buffer.end()) {
				if (use_encoding) {
					this->context = nst::leb128::read_unsigned64(reader, buffer.end());

				}
				else {
					reader = nst::primitive::read(this->context, reader);
				}
				reader = name.read(buffer, reader);
			}
			return reader;
		}
		nst::u32 stored() const {
			if (use_encoding) {
				return nst::leb128::unsigned_size(this->context) + name.stored() ;
			}
			else {
				return sizeof(this->context) + name.stored();
			}
			
		}
		nst::buffer_type::iterator store(nst::buffer_type::iterator& w) const {
			nst::buffer_type::iterator writer = w;
			if (use_encoding) {
				writer = nst::leb128::write_unsigned(writer, this->context);
			}
			else {
				writer = nst::primitive::store(writer, this->context);
			}
			
			
			writer = name.store(writer);
			if (writer - w != stored()) {
				ptrdiff_t diff = writer - w;
				ptrdiff_t stred = stored();
				err_print("wrote wrong byte count");
			}
			return writer;
		}
		size_t hash() const {
			return context*31 + name.hash();
		}
		
	};
	class record {
	private:
		ui4 flags;
		ui8 identity;
		// value section
		data value;

	public:
		MSGPACK_DEFINE_ARRAY(flags,identity,value)

		enum FLAGS{
			FLAG_LARGE = 0,
			FLAG_ROUTE = 1
		};

		static const bool use_encoding = true;
		record(const record&r) {
			*this = r;
		}
		record() {
			identity = 0;
			flags = 0;
		}
		void set_flag(FLAGS flag){
			this->flags |= (1ul << (ui4)flag);
		}
		void clear_flag(FLAGS flag){
			this->flags |= ~(1ul << (ui4)flag) ;

		}
		bool is_flag(FLAGS flag) const {
			return (this->flags & (1ul << (ui4)flag)) != 0;
		}
		record& operator=(const record& r) {

			value = r.value;
			identity = r.identity;
            flags = r.flags;
			return *this;
		}

		ui8 get_identity() const {
			return this->identity;
		}
		void set_identity(ui8 identity) {
			this->identity = identity;
		}

		data& get_value() {
			return value;
		}
		const data& get_value() const {
			return value;
		}

		ui4 size() const{
			return value.size();
		}

	public:

		/// persistence functions
		nst::buffer_type::const_iterator read(const nst::buffer_type& buffer, typename nst::buffer_type::const_iterator& r) {
			nst::buffer_type::const_iterator reader = r;

			if (reader != buffer.end()) {
				if (use_encoding) {
					this->flags = nst::leb128::read_unsigned64(reader, buffer.end());
					this->identity = nst::leb128::read_unsigned64(reader, buffer.end());

				}
				else {
					reader = nst::primitive::read(this->flags, reader);
					reader = nst::primitive::read(this->identity, reader);
				}

				reader = value.read(buffer, reader);
			}
			return reader;
		}
		nst::u32 stored() const {
			if (use_encoding) {
				return nst::leb128::unsigned_size(this->flags) + nst::leb128::unsigned_size(this->identity) + value.stored() ;
			}
			else {
				return sizeof(this->flags) + sizeof(this->identity) + value.stored();
			}

		}
		nst::buffer_type::iterator store(nst::buffer_type::iterator& w) const {

			nst::buffer_type::iterator writer = w;
			if (use_encoding) {
				writer = nst::leb128::write_unsigned(writer, this->flags);
				writer = nst::leb128::write_unsigned(writer, this->identity);
			}
			else {
				writer = nst::primitive::store(writer, this->flags);
				writer = nst::primitive::store(writer, this->identity);
			}
			writer = value.store(writer);
			if (writer - w != stored()) {
				ptrdiff_t diff = writer - w;
				ptrdiff_t stred = stored();
				err_print("wrote wrong byte count");
			}
			return writer;
		}

	};
	typedef struct lua_space{
		lua_space(){

		}
		lua_space(key k, record r) : first(k),second(r){

		}
		~lua_space(){

		}
		key first;
		record second;
	} space;
}
namespace rabbit{
	template<>
	struct rabbit_hash<spaces::key>{
		size_t operator()(const spaces::key& k) const{
			return k.hash(); ///
		};
	};
}
namespace std{
	template<>
	struct hash<spaces::key>{
		size_t operator()(const spaces::key& k) const{
			return k.hash(); ///
		};
	};
}
namespace stx{
	template<>
	struct btree_hash<spaces::key>{
		size_t operator()(const spaces::key& k) const{
			return k.hash(); ///
		};
	};
}