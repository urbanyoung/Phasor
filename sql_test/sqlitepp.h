#pragma once
#pragma warning( disable : 4290 )

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include "../sqlite/sqlite3.h"

/* Simple SQLite wrapper supporting prepared parameter binding for select
 * and insert operations. */
namespace sqlite 
{
	class SQLiteObject;
	class SQLiteRow;
	class SQLiteResult;
	class SQLiteValue;
	class SQLiteQuery;

	/* Error codes */
	enum SQLPP_ERRCODE
	{
		SQL_ERRCODE_TYPE = 1, // Attempting to access data with unexpected type
		SQL_ERRCODE_NO_INIT, // Attempting to use data that hasn't been initialized
		SQL_ERRCODE_NO_PARAM, // Attempting to bind to a non existent parameter
		SQL_ERRCODE_MISUSE, // The API was used incorrectly
		SQL_ERRCODE_UNK // unknown error
	};

	/* Types of data that can be stored in SQLiteValue */
	enum VALUE_TYPES 
	{
		TYPE_STRING = 0,
		TYPE_WSTRING,
		TYPE_INT,
		TYPE_DOUBLE
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteError
	// Purpose: Exception class used by this namespace
	class SQLiteError : public std::exception
	{
	private:
		SQLPP_ERRCODE err;
		std::stringstream* msg;

	public:
		SQLiteError(SQLPP_ERRCODE error, const char* sql_desc=NULL);
		~SQLiteError();
		virtual const char* what() const throw();
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

		/* Removes the object from tracking list */
		void untrack_object(SQLiteObject* obj);

	public:
		SQLite(std::string path_to_db);
		~SQLite();

		/* Creates a new query */
		SQLiteQuery* NewQuery(const char* query);

		/* Frees an object returned by the public interface */
		void free_object(SQLiteObject* obj);

		sqlite3* GetSQLite() { return sqlhandle; }

		friend class SQLiteObject;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteObject
	// Purpose: Base class for objects returned to the user
	class SQLiteObject
	{
	protected:
		SQLite* parent;

		/* Associates the object with a "parent" so it can be untracked 
		 * when deleted */
		void SetParent(SQLite* parent) {this->parent = parent; }

	public:
		SQLiteObject() { parent = NULL; }
		virtual ~SQLiteObject();

		friend class SQLite;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteQuery
	// Purpose: Provide an interface for executing prepared queries
	class SQLiteQuery : public SQLiteObject
	{
	private:
		sqlite3_stmt* stmt;
		char* query;

		/* Allocates memory for and copies over the query. If there is an
		 * existing query it is freed before copying the new one. */
		void copy_query(const char* query);

		/* Prepares the statement */
		void prepare_statement() throw(SQLiteError);

		SQLiteQuery(SQLite* parent, const char* query) throw(SQLiteError);
		virtual ~SQLiteQuery();
	public:	

		/* Executes the statement */
		void Execute(SQLiteResult* result=NULL) throw(SQLiteError);
		/* Resets the statement to use the given query */
		void Reset(const char* query) throw(SQLiteError);
		/* Binds values to the required parameters */
		void BindValue(const char* name, SQLiteValue value) throw(SQLiteError);
		void BindValue(int index, SQLiteValue value) throw(SQLiteError);
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteValue
	// Purpose: Represents data for exchange between program and database.
	class SQLiteValue : public SQLiteObject
	{
	private:
		VALUE_TYPES type;
				
		// stores a pointer to the data contained (could be of varying types)
		void* data_ptr;

		// Ensures the type of data stored is what's expected
		inline void CheckType(VALUE_TYPES expected) throw(SQLiteError) {
			if (expected != type) throw SQLiteError(SQL_ERRCODE_TYPE);
		}

	public:
		
		SQLiteValue(std::string val);
		SQLiteValue(std::wstring val);
		SQLiteValue(int val);
		SQLiteValue(double val);
		SQLiteValue(bool val);
		virtual ~SQLiteValue();

		/*	Get the row data in various data types, if the requested type
		 *	is not stored in this row a SQLiteTypeError exception is thrown
		 */
		const std::string GetStr();
		const std::wstring GetWStr();
		const int GetInt();
		const double GetDouble();

		int GetType() { return type; }
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