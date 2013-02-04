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
	bool do_track = true;

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
		std::string err_msg;
		std::unique_ptr<COutStream> stream;

	public:
		CAliasEvent(std::unique_ptr<COutStream> stream, 
			e_alias_events event_type, const std::string& hash, 
			const std::wstring& player_name, const std::string& wildcard) 
		: PhasorThreadEvent(0), event_type(event_type), hash(hash),
				player_name(player_name), wildcard(wildcard), has_error(false),
				result(nullptr), stream(std::move(stream))
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
							aliasdb->NewQuery(L"SELECT * FROM alias WHERE name LIKE :wildcard ORDER BY hash,name");
						query->BindValue(":wildcard", wildcard);
						this->result = std::move(query->Execute());
						ReinvokeInMain(g_Thread);
					} break;
				}
			}
			catch (SQLiteError& error)
			{
				err_msg = error.what();
				has_error = true;
			}
		}

		virtual void OnEventMain(PhasorThread& thread)
		{
			if (has_error) {
				*stream << "An error occurred and your query wasn't successful. "
					<< "Check the Phasor log for details" << endl;
				*g_PhasorLog << "ALIAS QUERY ERROR : " << err_msg << endl;
				return;
			}

			switch (event_type)
			{
			case kWildcardQuery:
				{
					for (size_t x = 0; x < result->size(); x++) {
						ObjWString& name = (ObjWString&)(*result)[x]["name"];
						ObjWString& hash = (ObjWString&)(*result)[x]["hash"];
						*stream << name.GetValue() << " - " << hash.GetValue()
							<< endl;						
					}

					stream->print("%i results with names matching '%s'", 
						result->size(), wildcard.c_str());
				} break;

			case kHashQuery:
				{
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
						*stream << line << endl;
						x += process_count;
					}
					stream->print("%i results matching hash '%s'", 
							result->size(), hash.c_str());
						
				} break;
			}
		}
	};

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
			g_PhasorLog->print("Cannot create alias database, error '%s'", error.what());
			aliasdb.reset();
		}
	}

	void OnPlayerJoin(s_player& player)
	{
		if (!do_track) return;
		if (aliasdb != nullptr) {
			std::shared_ptr<PhasorThreadEvent> e(
				new CAliasEvent(g_PrintStream.clone(), kSaveAlias, player.hash,
				player.mem->playerName,	""));
			g_Thread.InvokeInAux(e); // sole ownership by g_Thread once returned
		}
	}

	e_command_result DisplayAliasFailedToInitiliaze(COutStream& out)
	{
		out << "The alias system failed to initialize at startup. No commands can be used." << endl;
		return e_command_result::kProcessed;
	}

	// --------------------------------------------------------------------
	e_command_result sv_alias_search(void*, CArgParser& args, COutStream& out)
	{
		if (aliasdb == nullptr) return DisplayAliasFailedToInitiliaze(out);
		std::string query = args.ReadString();
		std::shared_ptr<PhasorThreadEvent> e(
			new CAliasEvent(out.clone(), kWildcardQuery, "", L"", query));
		g_Thread.InvokeInAux(e);
		out << "Fetching results. Please wait." << endl;

		return e_command_result::kProcessed;
	}

	e_command_result sv_alias_hash(void*, CArgParser& args, COutStream& out)
	{
		if (aliasdb == nullptr) return DisplayAliasFailedToInitiliaze(out);

		std::string hash = args.ReadPlayerOrHash();
		std::shared_ptr<PhasorThreadEvent> e(
			new CAliasEvent(out.clone(), kHashQuery, hash, L"", ""));
		g_Thread.InvokeInAux(e);
		out << "Fetching results. Please wait." << endl;
		return e_command_result::kProcessed;
	}

	e_command_result sv_alias_enable(void*, CArgParser& args, COutStream& out)
	{
		if (aliasdb == nullptr) return DisplayAliasFailedToInitiliaze(out);
		do_track = args.ReadBool();
		out << "The alias system is " << (do_track ? "active" : "inactive") << endl;
		return e_command_result::kProcessed;
	}
}}