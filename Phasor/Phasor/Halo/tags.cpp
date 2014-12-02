#include "tags.h"
#include <map>
#include "Addresses.h"

namespace halo
{
#define ENDIAN_SWAP32(tmp) (tmp >> 24) |\
	((tmp >> 8) & 0x0000ff00) |\
		((tmp << 8) & 0x00ff0000) | \
		(tmp << 24)

	s_tag_type::s_tag_type(const char* str)
	{
		unsigned long tmp = *(unsigned long*)str;
		val = ENDIAN_SWAP32(tmp);
	}

	bool s_tag_type::operator==(const s_tag_type& other) {	return other.val == val; }
	bool s_tag_type::operator==(e_tag_types type) { return (unsigned long)type == val; }
	bool s_tag_type::operator!=(const s_tag_type& other) { return !(*this == other); }
	bool s_tag_type::operator!=(e_tag_types type){ return !(*this == type); }

	char* s_tag_type::GetString(char out[5])
	{
		*(unsigned long*)out = ENDIAN_SWAP32(val);
		out[4] = '\0';
		return out;
	}
	// -------------------------------------------------------------------
	//
	std::map<std::string, s_tag_entry*> tag_cache;
	s_tag_entry* LookupTag(ident tagId)
	{
		s_tag_index_table_header* tag_table = *(s_tag_index_table_header**)ADDR_TAGTABLE;
		if (tagId.slot > tag_table->entityCount) return 0;
        if (tag_table->next_ptr[tagId.slot].id != tagId) return 0;
		return &tag_table->next_ptr[tagId.slot];
	}

	std::string GetTagCacheKey(s_tag_type type, const std::string& tag_name)
	{
		char tag_str[6];
		type.GetString(tag_str);
		tag_str[4] = '\\';
		tag_str[5] = 0;

		return tag_name + std::string(tag_str);
	}

	// std::map averaged about ~20 ticks in QueryPerformanceCounter faster
	// than linear search in release mode.
	s_tag_entry* LookupTag(s_tag_type type, const std::string& tag_name)
	{
		auto itr = tag_cache.find(GetTagCacheKey(type, tag_name));
		if (itr == tag_cache.end()) return 0;
		return itr->second;
	}

	void BuildTagCache()
	{
		tag_cache.clear();

		s_tag_index_table_header* tag_table = *(s_tag_index_table_header**)ADDR_TAGTABLE;
		s_tag_entry* tag = tag_table->next_ptr;
		for (size_t x = 0; x < tag_table->entityCount; x++, tag++) {
			tag_cache.insert(std::pair<std::string, s_tag_entry*>
				(GetTagCacheKey(tag->tagType, tag->tagName), tag));
		}
	}
}