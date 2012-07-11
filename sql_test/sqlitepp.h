#pragma once

#include <string>
#include <vector>
#include <set>
#include "../sqlite/sqlite3.h"

/* Simple SQLite wrapper supporting prepared parameter binding for select
 * and insert operations. */
namespace sqlite 
{
	class SQLiteObject;
	class SQLiteRow;
	class SQLiteResult;
	class SQLiteValue;

	/* Error codes */
	enum SQLPP_ERRCODE
	{
		SQL_ERRCODE_TYPE = 1
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteError
	// Purpose: Exception class used by this namespace
	class SQLiteError : public std::exception
	{
	private:
		SQLPP_ERRCODE err;
		std::string desc;

	public:
		SQLiteError(SQLPP_ERRCODE error);
		virtual const char* what() const throw()
		{ return desc.c_str(); }
		const int type() { return err; }
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLite
	// Purpose: Provide a wrapper around the sqlite3 C interface. 
	class SQLite
	{
	private:
		std::set<SQLiteObject*> allocated_objects;
		sqlite3* sqlhandle;

		sqlite3* GetSQLite() { return sqlhandle; }

	public:
		SQLite(std::string path_to_db);
		~SQLite();

		/* Frees an object returned by the public interface */
		void free_object(SQLiteObject* obj);

		friend class SQLiteQuery;
	};

	class SQLiteQuery
	{
	private:
		sqlite3_stmt* stmt;
		SQLite* sqlcon;

	public:
		SQLiteQuery(SQLite* sqlcon);
		~SQLiteQuery();

		bool Execute(const char* query, SQLiteResult* result=NULL) 
			throw(SQLiteError);

		void Prepare(const char* query, const char* name, SQLiteValue value);
		void Prepare(const char* query, int index, SQLiteValue value);
		
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteObject
	// Purpose: Base class for objects returned to the user
	class SQLiteObject
	{
	private:
		SQLiteObject();
		virtual ~SQLiteObject();

		friend class SQLite;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteValue
	// Purpose: Represents data for exchange between program and database.
	class SQLiteValue : public SQLiteObject
	{
	private:
		enum VALUE_TYPES 
		{
			TYPE_STRING = 0,
			TYPE_WSTRING,
			TYPE_INT,
			TYPE_DOUBLE,
			TYPE_FLOAT,
			TYPE_BOOL
		};
		// stores a pointer to the data contained (could be of varying types)
		void* data_ptr;

		// Data type being stored
		VALUE_TYPES type;

		// Ensures the type of data stored is what's expected
		inline void CheckType(VALUE_TYPES expected) throw(SQLiteError) {
			if (expected != type) throw SQLiteError(SQL_ERRCODE_TYPE);
		}

	public:
		
		SQLiteValue(std::string val);
		SQLiteValue(std::wstring val);
		SQLiteValue(int val);
		SQLiteValue(double val);
		SQLiteValue(float val);
		SQLiteValue(bool val);
		virtual ~SQLiteValue();

		/*	Get the row data in various data types, if the requested type
		 *	is not stored in this row a SQLiteTypeError exception is thrown
		 */
		const std::string GetStr();
		const std::wstring WGetStr();
		const int GetInt();
		const double GetDouble();
		const float GetFloat();
		const bool GetBool();
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteRow
	// Purpose: Represent a row of data returned via SELECT statements
	class SQLiteRow : public SQLiteObject
	{
	private:
		std::vector<SQLiteValue*> columns;

		SQLiteRow();
		virtual ~SQLiteRow();
	public:
		
		SQLiteValue* operator [] (const unsigned int index) 
			throw(SQLiteError);
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteResult
	// Purpose: Represent a data set returned via SELECT statements
	class SQLiteResult : public SQLiteObject
	{
	private: 
		std::vector<SQLiteRow*> result;
		
		SQLiteResult();
		virtual ~SQLiteResult();

	public:

		SQLiteRow* operator [] (const unsigned int index)
			throw(SQLiteError);
	};
}