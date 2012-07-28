#pragma once

#include <string>
#include <map>
#include <windows.h>

namespace Phasor
{
	class Error
	{
	private:
		std::string err;
		bool hasErr;

	protected:
		void SetError(const std::string& error);

	public:
		Error();
		~Error();
		std::string GetError() const;
		bool hasError() const;
	};
	
	class Logging
	{
	private:
		// stores open logs and their associated ids
		static std::map<int, Logging*> logs;
		DWORD exec_thread; // if non-null saving occurs in that thread

		Logging();
		~Logging();

		void LogData(const std::string& type, const std::string& name,
			const char* _Details, ...);
		void LogData(const std::string& type, const std::string& name,
			const wchar_t* _Details, ...);

	public:
		static void Create(int id, const std::string& file);
		static void Close(int id);
		static void SetMaxSize(int id, size_t size);
		static size_t GetMaxSize(int id);

		static void LogData(int id, const std::string& type, const std::string& name,
			const char* _Details, ...);
		static void LogData(int id, const std::string& type, const std::string& name,
			const wchar_t* _Details, ...);		
	};

	// Logs the error and then closes the server
	void AbortableError();

	// --------------------------------------------------------------------
	// 
	// Directory stuff
	bool SetupDirectories();
	std::string GetMapDirectory();
	std::string GetWorkingDirectory();
	std::string GetDataDirectory();
	std::string GetScriptDirectory();

	
}