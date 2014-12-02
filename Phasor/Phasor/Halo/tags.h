#pragma once

#include "Halo.h"
#include <string>

namespace halo
{
	enum e_tag_types
	{
		TAG_ACTV = 'actv',
		TAG_ANT  = 'ant!',
		TAG_ANTR = 'antr',
		TAG_BIPD = 'bipd',
		TAG_BTIM = 'bitm',
		TAG_COLL = 'coll',
		TAG_COLO = 'colo',
		TAG_CONT = 'cont',
		TAG_DECA = 'deca',
		TAG_DELA = 'DeLa',
		TAG_DEVC = 'devc',
		TAG_EFFE = 'effe',
		TAG_EQIP = 'eqip',
		TAG_FLAG = 'flag',
		TAG_FONT = 'font',
		TAG_FOOT = 'foot',
		TAG_GRHI = 'grhi',
		TAG_HMT = 'hmt ',
		TAG_HUD = 'hud#',
		TAG_HUDG = 'hudg',
		TAG_ITMC = 'itmc',
		TAG_JPT = 'jpt!',
		TAG_LENS = 'lens',
		TAG_LIGH = 'ligh',
		TAG_LSND = 'lsnd',
		TAG_MATG = 'matg',
		TAG_METR = 'metr',
		TAG_MGS2 = 'mgs2',
		TAG_MOD2 = 'mod2',
		TAG_PART = 'part',
		TAG_PCTL = 'pctl',
		TAG_PHYS = 'phys',
		TAG_PPHY = 'pphy',
		TAG_PROJ = 'proj',
		TAG_SBSP = 'sbsp',
		TAG_SCEN = 'scen',
		TAG_SCEX = 'scex',
		TAG_SCHI = 'schi',
		TAG_SCNR = 'scnr',
		TAG_SENV = 'senv',
		TAG_SGLA = 'sgla',
		TAG_SKY = 'sky ',
		TAG_SMET = 'smet',
		TAG_SND = 'snd!',
		TAG_SNDE = 'snde',
		TAG_SOUL = 'Soul',
		TAG_SPLA = 'spla',
		TAG_SSCE = 'ssce',
		TAG_STR = 'str#',
		TAG_TAGC = 'tagc',
		TAG_TRAK = 'trak',
		TAG_UDLG = 'udlg',
		TAG_UNHI = 'unhi',
		TAG_USTR = 'ustr',
		TAG_VEHI = 'vehi',
		TAG_WEAP = 'weap',
		TAG_WIND = 'wind',
		TAG_WPHI = 'wphi'
	};
	
	#pragma pack(push, 1)
	struct s_tag_type
	{
		unsigned long val;
		explicit s_tag_type(const char* str);

		bool operator==(const s_tag_type& other);
		bool operator==(e_tag_types type);
		bool operator!=(const s_tag_type& other);
		bool operator!=(e_tag_types type);
		char* GetString(char out[5]);
	};
	static_assert(sizeof(s_tag_type) == 0x4, "bad");

	// Structure of the tag header (entry in tag table)
	struct s_tag_entry
	{
		s_tag_type tagType; // ie weap
		UNKNOWN(8);
		ident id; // unique id for map
		const char* tagName; // name of tag
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

	s_tag_entry* LookupTag(ident tagId);
	s_tag_entry* LookupTag(s_tag_type type, const std::string& tag_name);
	void BuildTagCache(); // call at start of each new game.
}