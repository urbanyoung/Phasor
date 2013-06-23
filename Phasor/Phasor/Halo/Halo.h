#pragma once

namespace halo
{
#define STR_CAT(a,b)            a##b
#define STR_CAT_DELAYED(a,b)   STR_CAT(a,b)
#define UNKNOWN(size) char STR_CAT_DELAYED(_unused_,__COUNTER__)[size]
#define UNKNOWN_BITFIELD(size) char STR_CAT_DELAYED(_unusedbf_, __COUNTER__) : size

#pragma pack(push, 1)

	struct ident 
	{
		unsigned short slot;
		unsigned short id;		

		explicit ident() {
			slot = 0xFFFF;
			id = 0xFFFF;
		}

		bool operator<(const ident& other)
		{
			return (unsigned long)*this < (unsigned long)other;
		}

		bool operator>(const ident& other)
		{
			return (unsigned long)*this > (unsigned long)other;
		}

		operator unsigned long() const
		{
			unsigned long result = id << 16 | (slot & 0xffff);
			return result;
		}

		bool valid() const { return id != 0xFFFF || slot != 0xFFFF; }
	};
	static_assert(sizeof(ident) == 4, "bad");

	ident make_ident(unsigned long id); 

	// Header for tables used throughout Halo (objects, players, loopobjects)
	struct s_table_header
	{
		char name[0x20];
		unsigned short max_size;
		unsigned short elem_size;
		UNKNOWN(4); // only seen as 1
		UNKNOWN(4); // d@t@
		UNKNOWN(2);
		unsigned short cur_items; // i think
		UNKNOWN(4);
		void* data;
	};
	static_assert(sizeof(s_table_header) == 0x38, "s_table_header size incorrect");

#pragma pack(pop)
}