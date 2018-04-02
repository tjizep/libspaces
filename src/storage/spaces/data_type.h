#pragma once
#include <stx/storage/types.h>
namespace spaces{
	struct data_type_ext{
		enum
		{
			final = 0
			, nil
			, boolean
			, integer8
			, integer16
			, integer32
			, integer64
			, real
			, char_sequence
			, char_sequence8
			, pair_sequence
			, decimal
			, infinity
		};
	};	
};
typedef stx::storage::i8	i1;
typedef stx::storage::u8	ui1;
typedef stx::storage::i32	i4;
typedef stx::storage::i64	i8;
typedef stx::storage::u32	ui4;
typedef stx::storage::u64	ui8;
typedef stx::storage::f64	f8;
typedef size_t umi;
#define DEFINE_SESSION_KEY(key) static umi _spaces_##key = 0;	ui8 key = (umi)(void*)(&_spaces_##key);
#define INCLUDE_SESSION_KEY(key) extern ui8 key ;