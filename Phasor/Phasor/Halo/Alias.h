#pragma once

#include "Player.h"
#include "../Commands.h"
#include "../PhasorThread.h"
#include "../../Libraries/sqlitepp.h"
#include <string>

namespace halo { namespace alias 
{
	class CAliasError : public std::runtime_error
	{
	public:
		CAliasError(const char* msg) : std::runtime_error(msg) {}
	};

	class CAliasEvent : public PhasorThreadEvent
	{
	private:	
		std::unique_ptr<sqlite::SQLiteResult> result;
		bool has_error;
		std::string err_msg;

		void HasResult();
		void SaveError(const char* err);

	protected:
		std::unique_ptr<COutStream> stream;

	public:
		CAliasEvent(std::unique_ptr<COutStream> stream);
		virtual ~CAliasEvent();

		// return true to indicate there's a result
		virtual bool ProcessQuery() = 0;
		virtual void ProcessResult();

		// inherited from PhasorThread
		virtual void OnEventAux(PhasorThread& thread) override;
		virtual void OnEventMain(PhasorThread& thread) override;
	};

	class AliasEventAddPlayer : public CAliasEvent
	{
	private:
		std::string hash;
		std::wstring name;

	public:
		AliasEventAddPlayer(std::unique_ptr<COutStream> stream,
			const std::string& hash, const std::wstring& name);
		virtual bool ProcessQuery() override;
	};

	class AliasEventQueryHash : public CAliasEvent
	{
	private:
		std::string hash;
	protected:
		std::unique_ptr<sqlite::SQLiteResult> result;

	public:
		AliasEventQueryHash(std::unique_ptr<COutStream> out, const std::string& hash);
		bool ProcessQuery() override;
		void ProcessResult() override;
	};

	class AliasEventQueryPlayer : public CAliasEvent
	{
	private:
		std::string player;
	protected:
		std::unique_ptr<sqlite::SQLiteResult> result;

	public:
		AliasEventQueryPlayer(std::unique_ptr<COutStream> out, const std::string& player);
		bool ProcessQuery() override;
		void ProcessResult() override;
	};

	void Initialize();
	void OnPlayerJoin(s_player& player);
	bool ExecuteEvent(std::shared_ptr<CAliasEvent> e);
	
	// Server commands
	using namespace commands;
	e_command_result sv_alias_search(void*, CArgParser& args, COutStream& out);
	e_command_result sv_alias_hash(void*, CArgParser& args, COutStream& out);
	e_command_result sv_alias_enable(void*, CArgParser& args, COutStream& out);

}}