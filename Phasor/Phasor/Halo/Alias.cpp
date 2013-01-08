#pragma once

#include "Alias.h"
#include "../PhasorThread.h"
#include "../Globals.h"
#include "../../Libraries/sqlitepp.h"
#include "../../Common/MyString.h"
#include "../Directory.h"

namespace halo { namespace alias 
{
	using namespace sqlite;
	std::unique_ptr<SQLite> aliasdb;

	enum e_alias_events
	{
		kSaveAlias = 0,
		kHashQuery,
		kWildcardQuery,
	};

	class CAliasEvent : public PhasorThreadEvent
	{
	private:	
		std::string hash, wildcard;
		std::wstring player_name;
		e_alias_events event_type;
		std::unique_ptr<SQLiteResult> result;
		bool has_error;
		COutStream& stream;

	public:
		CAliasEvent(COutStream& stream, 
			e_alias_events event_type, const std::string& hash, 
			const std::wstring& player_name, const std::string& wildcard) 
		: PhasorThreadEvent(0), stream(stream), event_type(event_type), hash(hash),
				player_name(player_name), wildcard(wildcard), has_error(false),
				result(nullptr)
		{
		}

		virtual void OnEventAux(PhasorThread& thread)
		{
			try
			{
				switch (event_type)
				{
				case kSaveAlias:
					{
						std::unique_ptr<SQLiteQuery> query = 
							aliasdb->NewQuery(
							L"SELECT * FROM alias WHERE hash=:hash AND name=:name"
							);
						query->BindValue(":hash", hash);
						query->BindValue(":name", player_name);
						std::unique_ptr<SQLiteResult> result = query->Execute();

						if (result->size() == 0) {
							query->Reset(
								L"INSERT INTO alias (hash, name) VALUES(:hash,:name)");
							query->BindValue(":hash", hash);
							query->BindValue(":name", player_name);
							query->Execute();
						}
					} break;
				case kHashQuery:
					{
						std::unique_ptr<SQLiteQuery> query = 
							aliasdb->NewQuery(L"SELECT * FROM alias WHERE hash=:hash");
						query->BindValue(":hash", hash);
						this->result = std::move(query->Execute());
						ReinvokeInMain(g_Thread);
					} break;
				case kWildcardQuery:
					{
						std::unique_ptr<SQLiteQuery> query = 
							aliasdb->NewQuery(L"SELECT * FROM alias WHERE name LIKE :wildcard");
						query->BindValue(":wildcard", wildcard);
						this->result = std::move(query->Execute());
						ReinvokeInMain(g_Thread);
					} break;
				}
			}
			catch (SQLiteError& error)
			{
				g_PhasorLog->print("alias query failed : %s", error.what());
				has_error = true;
			}
		}

		virtual void OnEventMain(PhasorThread& thread)
		{
			if (has_error) {
				stream << "An error occurred and your query wasn't successful. "
					<< "Check the Phasor log for details" << endl;
				return;
			}

			switch (event_type)
			{
			case kWildcardQuery:
			case kHashQuery: // Print the results
				{
					// todo: send to correct stream
					static const int kNamesPerLine = 4;
					size_t x = 0;
					while (x < result->size()) {
						std::wstring line;
						line.reserve(80);
						size_t left = result->size() - x;
						size_t process_count = left < kNamesPerLine ? left : kNamesPerLine;
						for (size_t i = 0; i < process_count; i++) {
							ObjWString& name = (ObjWString&)(*result)[x + i]["name"];
							line += m_swprintf(L"%-19s", name.GetValue());
						}
						stream << line << endl;
						x += process_count;
					}
					if (event_type == kHashQuery)
						stream.print("%i results matching hash '%s'", 
							result->size(), hash.c_str());
					else if (event_type == kWildcardQuery)
						stream.print("%i results matching '%s'", 
							result->size(), wildcard.c_str());
				} break;
			}
		}
	};
	std::shared_ptr<CAliasEvent> threadEvent;

	void Initialize()
	{
		std::string alias_file = m_sprintf("%s\\alias.sqlite",
			NarrowString(g_DataDirectory).c_str());
		aliasdb.reset(new sqlite::SQLite(alias_file));
		try 
		{
			std::unique_ptr<SQLiteQuery> query = aliasdb->NewQuery(
				L"CREATE TABLE IF NOT EXISTS alias (hash CHAR(32), name VARCHAR(20))");
			query->Execute();
		}
		catch (SQLiteError& error)
		{
			g_PhasorLog->print("Cannot create alias database error '%s'", error.what());
			aliasdb.reset();
		}
	}

	void OnPlayerJoin(s_player& player)
	{
		if (aliasdb != nullptr) {
			std::shared_ptr<PhasorThreadEvent> e(
				new CAliasEvent(g_PrintStream, kSaveAlias, player.hash,
				player.mem->playerName,	""));
			g_Thread.InvokeInAux(e);
		}
	}

	// --------------------------------------------------------------------
	e_command_result sv_alias_search(void*, CArgParser& args, COutStream& out)
	{
		return e_command_result::kProcessed;
	}

	e_command_result sv_alias_hash(void*, CArgParser& args, COutStream& out)
	{
		std::string hash = args.ReadPlayerOrHash();
		if (aliasdb == nullptr) {
			out << "The alias system is inactive. Enable with sv_alias_enable" << endl;
			return e_command_result::kProcessed;
		}

		std::shared_ptr<PhasorThreadEvent> e(
			new CAliasEvent(out, kHashQuery, hash, L"", ""));
		g_Thread.InvokeInAux(e);

		return e_command_result::kProcessed;
	}

	e_command_result sv_alias_enable(void*, CArgParser& args, COutStream& out)
	{
		bool state = args.ReadBool();
		if (state && aliasdb != nullptr)
		return e_command_result::kProcessed;
	}
}}