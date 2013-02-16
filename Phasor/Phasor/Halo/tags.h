#pragma once

#include "Halo.h"
#include <string>

namespace halo
{
	#pragma pack(push, 1)
	struct s_tag_type
	{
		unsigned long val;
		s_tag_type(const char* str);
	};
	static_assert(sizeof(s_tag_type) == 0x4, "bad");

	// Structure of the tag header (entry in tag table)
	struct s_tag_entry
	{
		s_tag_type tagType; // ie weap
		UNKNOWN(8);
		ident id; // unique id for map
		char* tagName; // name of tag
		void* metaData; // data for this tag
		UNKNOWN(8);
	};
	static_assert(sizeof(s_tag_entry) == 0x20, "bad");

	// Structure of tag index table
	struct s_tag_index_table_header
	{
		s_tag_entry* next_ptr;
		unsigned long starting_ident; // ??
		UNKNOWN(4);
		unsigned long entityCount;
		UNKNOWN(4);
		unsigned long readOffset;
		UNKNOWN(8);
		unsigned long readSize;
		UNKNOWN(4);
	};
	static_assert(sizeof(s_tag_index_table_header) == 0x28, "bad");

	struct s_object_info
	{
		s_tag_type tagType;
		char* tagName;
		unsigned long empty;
		ident id;
	};

#pragma pack(pop)

	void* GetObjectAddress(ident objectId);
	s_tag_entry* LookupTag(ident tagId);
	s_tag_entry* LookupTag(s_tag_type type, const std::string& tag_name);
	void BuildTagCache(); // call at start of each new game.
}