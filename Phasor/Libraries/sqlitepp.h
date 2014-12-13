#pragma once

#include <string>
#include <vector>
#include <map>
#include "../../sqlite/sqlite3.h"
#include "../Common/Common.h"

#ifndef BUILD_DEBUG
#pragma comment(lib, "../release/sqlite.lib")
#else
#pragma comment(lib, "../debug/sqlite.lib")
#endif

namespace sqlite 
{
	using namespace Common;

	class SQLiteResult;
	class SQLiteRow;
	class SQLiteQuery;

	/* Error codes */
	enum sqlite_errs
	{
		SQL_ERRCODE_FAILED_OPEN, // sqlite3_open failed for some reason
		SQL_ERRCODE_NO_INIT, // Attempting to use data that hasn't been initialized
		SQL_ERRCODE_BAD_INDEX, // Attempted to access out of bounds data
		SQL_ERRCODE_NO_PARAM, // Attempting to bind to a non existent parameter
		SQL_ERRCODE_MISUSE, // The API was used incorrectly
		SQL_ERRCODE_CLOSED, // The database has been closed
		SQL_ERRCODE_UNK // unknown error
	};

	static const char* err_descs[] = {
		{"Unable to open the database file."},
		{"Attempted to use uninitialized data."},
		{"Out of bounds access attempt on SQLiteResult or SQLiteRow"},
		{"Attempted to bind to an unknown parameter"},
		{"api misuse"},
		{"The database has been closed and an operation was attempted."},
		{"Unknown error"}
	};

	/* Types of data that can be stored in SQLiteValue */
	enum sqlite_types
	{
		TYPE_STRING = 0,
		TYPE_WSTRING,
		TYPE_INT,
		TYPE_DOUBLE,
		TYPE_BLOB,
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteError
	// Purpose: Exception class used by this namespace
	class SQLiteError : public std::exception
	{
	public:
		SQLiteError(int error);
		SQLiteError(const char* e);
		virtual ~SQLiteError();
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLite
	// Purpose: Provide a basic wrapper around the sqlite3 C interface. 
	class SQLite
	{
	private:
		sqlite3* sqlhandle;

	public:
		SQLite(const std::string& path_to_db);
		~SQLite();

		// Closes the database connection
		void Close();

		// Creates a new query
		std::unique_ptr<SQLiteQuery> NewQuery(const std::wstring& query);

		// Returns a pointer to the open sqlite3 database. If the database
		// has been closed an exception is thrown.
		sqlite3* GetSQLite();

		// Gets the last error
		const char* LastError();

		friend class SQLiteQuery;
		friend class SQLiteObject;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteObject
	// Purpose: Base class for objects returned to the user which require
	// an associated state.
	class SQLiteObject
	{
	protected:
		SQLite& parent;

	public:
		SQLiteObject(SQLite& parent);
		virtual ~SQLiteObject();

		friend class SQLite;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteValue
	// Purpose: Wrapper for converting types
	struct SQLiteValue
	{		
		Object::unique_ptr object;
		sqlite_types type;

		SQLiteValue(const std::string& val);
		SQLiteValue(const std::wstring& val);
		SQLiteValue(const char* val);
		SQLiteValue(const wchar_t* val);
		SQLiteValue(int val);
		SQLiteValue(double val);
		SQLiteValue(BYTE* val, size_t length);

		friend class SQLiteRow;
	};


	//-----------------------------------------------------------------------------------------
	// Class: SQLiteQuery
	// Purpose: Provide an interface for executing prepared queries
	class SQLiteQuery : public SQLiteObject
	{
	private:
		sqlite3_stmt* stmt;
		std::wstring query;

		// Prepares the statement
		void prepare_statement();
		
	public:	

		SQLiteQuery(SQLite& parent, const std::wstring& query);
		virtual ~SQLiteQuery();

		// Executes the statement
		std::unique_ptr<SQLiteResult> Execute();

		// Resets the statement to use the given query
		void Reset(const std::wstring& query);

		// Binds values to the required parameters
		void BindValue(const char* name, const SQLiteValue& value);
		void BindValue(int index, const SQLiteValue& value);
	
		friend class SQLite;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteRow
	// Purpose: Represent a row of data returned
	class SQLiteRow
	{
	private:
		typedef std::pair<std::string, Object::unique_ptr> pair_t;
		std::map<std::string, Object::unique_ptr> columns;

		/* Adds data to the row */
		void AddColumn(Object::unique_ptr value,
			const std::string& name);
	public:

		Object& operator[] (const std::string& key);
		Object& get(const std::string& key);

		/* Returns the number of items in the row*/
		size_t size();

		friend class SQLiteQuery;
		friend class SQLiteResult;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteResult
	// Purpose: Represent a data set returned via SELECT statements
	class SQLiteResult
	{
	private: 
		// used to store the retrieved rows 
		std::vector<std::unique_ptr<SQLiteRow>> rows;
		
		// Adds a new row to the result
		void AddRow(std::unique_ptr<SQLiteRow> row);
		
	public:

		// Returns the row at position i.
		SQLiteRow& operator[] (size_t i);
		SQLiteRow& get(size_t i);

		size_t size();
		friend class SQLiteQuery;
	};
}