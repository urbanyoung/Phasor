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

	// --------------------------------------------------------------------
	// CAliasEvent
	void CAliasEvent::HasResult()
	{
		ReinvokeInMain(g_Thread);
	}

	void CAliasEvent::SaveError(const char* err)
	{
		err_msg = err;
		has_error = true;
		HasResult(); // even if no result expected we want to process error
	}

	CAliasEvent::CAliasEvent(std::unique_ptr<COutStream> stream) 
	: PhasorThreadEvent(0), has_error(false), result(nullptr), 
		stream(std::move(stream))
	{
	}

	CAliasEvent::~CAliasEvent() {}

	void CAliasEvent::ProcessResult() {}

	void CAliasEvent::OnEventAux(PhasorThread& thread)
	{
		try
		{
			if (ProcessQuery()) HasResult();
		}
		catch (SQLiteError& error) {
			SaveError(error.what());
		} 
		catch (CAliasError& error) {
			SaveError(error.what());
		}
	}

	void CAliasEvent::OnEventMain(PhasorThread& thread)
	{
		if (has_error) {
			*stream << "An error occurred and your query wasn't successful. "
				<< "Check the Phasor log for details" << endl;
			*g_PhasorLog << "ALIAS QUERY ERROR : " << err_msg << endl;
			return;
		}
		ProcessResult();
	}

	// --------------------------------------------------------------------
	// 
	AliasEventAddPlayer::AliasEventAddPlayer(std::unique_ptr<COutStream> stream,
		const std::string& hash, const std::wstring& name)
		: CAliasEvent(std::move(stream)), hash(hash), name(name)
	{
	}

	bool AliasEventAddPlayer::ProcessQuery()
	{
		std::unique_ptr<SQLiteQuery> query = 
			aliasdb->NewQuery(
			L"SELECT * FROM alias WHERE hash=:hash AND name=:name"
			);
		query->BindValue(":hash", hash);
		query->BindValue(":name", name);
		std::unique_ptr<SQLiteResult> result = query->Execute();

		if (result->size() == 0) {
			query->Reset(
				L"INSERT INTO alias (hash, name) VALUES(:hash,:name)");
			query->BindValue(":hash", hash);
			query->BindValue(":name", name);
			query->Execute();
		}
		return false; // no result
	}

	// -------------------------------------------------------------------
	//
	AliasEventQueryHash::AliasEventQueryHash(std::unique_ptr<COutStream> out, 
		const std::string& hash)
		: CAliasEvent(std::move(out)), hash(hash) 
	{}

	bool AliasEventQueryHash::ProcessQuery()
	{
		std::unique_ptr<SQLiteQuery> query = 
			aliasdb->NewQuery(L"SELECT * FROM alias WHERE hash=:hash");
		query->BindValue(":hash", hash);
		this->result = std::move(query->Execute());
		return true; // has result
	}

	void AliasEventQueryHash::ProcessResult()
	{
		static const int kNamesPerLine = 4;
		size_t x = 0;
		while (x < result->size()) {
			std::wstring line;
			line.reserve(80);
			size_t left = result->size() - x;
			size_t process_count = left < kNamesPerLine ? left : kNamesPerLine;
			for (size_t i = 0; i < process_count; i++) {
				ObjWString& name = static_cast<ObjWString&>((*result)[x + i]["name"]);
				line += m_swprintf(L"%-19s", name.GetValue());
			}
			*stream << line << endl;
			x += process_count;
		}
		stream->print("%i results matching hash '%s'", 
			result->size(), hash.c_str());	
	}

	// --------------------------------------------------------------------
	//
	AliasEventQueryPlayer::AliasEventQueryPlayer(std::unique_ptr<COutStream> out, 
		const std::string& player)
		: CAliasEvent(std::move(out)), player(player)
	{}

	bool AliasEventQueryPlayer::ProcessQuery()
	{
		std::unique_ptr<SQLiteQuery> query = 
			aliasdb->NewQuery(L"SELECT * FROM alias WHERE name LIKE :wildcard ORDER BY hash,name");
		query->BindValue(":wildcard", player);
		this->result = std::move(query->Execute());
		return true;
	}

	void AliasEventQueryPlayer::ProcessResult()
	{
		for (size_t x = 0; x < result->size(); x++) {
			ObjWString& name = static_cast<ObjWString&>((*result)[x]["name"]);
			ObjWString& hash = static_cast<ObjWString&>((*result)[x]["hash"]);
			*stream << name.GetValue() << " - " << hash.GetValue()
				<< endl;						
		}

		stream->print("%i results with names matching '%s'", 
			result->size(), player.c_str());
	}
	
	// -------------------------------------------------------------------
	// Interface with rest of Phasor.
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
			std::shared_ptr<CAliasEvent> e(new AliasEventAddPlayer(g_PrintStream->clone(), 
				player.hash, player.mem->playerName));
			(void)ExecuteEvent(e);
		}
	}

	bool ExecuteEvent(std::shared_ptr<CAliasEvent> e)
	{
		if (aliasdb == nullptr) return false;
		std::shared_ptr<PhasorThreadEvent> thread_event = 
			std::static_pointer_cast<PhasorThreadEvent>(e);
		g_Thread.InvokeInAux(thread_event);
		return true;
	}

	// --------------------------------------------------------------------
	// Server commands
	// 

	e_command_result DisplayAliasFailedToInitiliaze(COutStream& out)
	{
		out << "The alias system failed to initialize at startup. No commands can be used." << endl;
		return e_command_result::kProcessed;
	}

	e_command_result sv_alias_search(void*, CArgParser& args, COutStream& out)
	{
		if (aliasdb == nullptr) return DisplayAliasFailedToInitiliaze(out);
		std::string query = args.ReadString();
		std::shared_ptr<CAliasEvent> e(new AliasEventQueryPlayer(out.clone(), query));
		(void)ExecuteEvent(e); // won't fail we do check above

		out << "Fetching results. Please wait." << endl;

		return e_command_result::kProcessed;
	}

	e_command_result sv_alias_hash(void*, CArgParser& args, COutStream& out)
	{
		if (aliasdb == nullptr) return DisplayAliasFailedToInitiliaze(out);
		std::string hash = args.ReadPlayerOrHash();

		std::shared_ptr<CAliasEvent> e(new AliasEventQueryHash(out.clone(), hash));
		(void)ExecuteEvent(e); // won't fail we do check above

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