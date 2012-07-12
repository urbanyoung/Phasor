#pragma once
#pragma warning( disable : 4290 )

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <windows.h>
#include "../sqlite/sqlite3.h"

#pragma comment(lib, "../release/sqlite.lib")

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
		SQL_ERR_NONE = 0,
		SQL_ERRCODE_FAILED_OPEN, // sqlite3_open failed for some reason
		SQL_ERRCODE_TYPE, // Attempting to access data with unexpected type
		SQL_ERRCODE_NO_INIT, // Attempting to use data that hasn't been initialized
		SQL_ERRCODE_NO_PARAM, // Attempting to bind to a non existent parameter
		SQL_ERRCODE_BAD_INDEX, // Attempted to access data that doesn't exist (SQLiteResult/Row)
		SQL_ERRCODE_MISUSE, // The API was used incorrectly
		SQL_ERRCODE_NDE, // sqlite3 returned data but none was expected
		SQL_ERRCODE_UNK // unknown error
	};

	/* Types of data that can be stored in SQLiteValue */
	enum VALUE_TYPES 
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
	private:
		int err;
		std::string msg;

	public:
		SQLiteError(int error, const char* sql_desc=NULL);
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
		bool alive; /* used when we're cleaning up to stop iterator corruption */
		sqlite3* sqlhandle;

		/* Removes the object from tracking list */
		void untrack_object(SQLiteObject* obj);

		/* Tracks a new object */
		void track_object(SQLiteObject* obj);

	public:
		SQLite(std::string path_to_db) throw(SQLiteError);
		~SQLite();

		/* Creates a new query */
		SQLiteQuery* NewQuery(const char* query) throw(SQLiteError);

		/* Frees an object returned by the public interface */
		void free_object(SQLiteObject* obj);

		sqlite3* GetSQLite() { return sqlhandle; }

		friend class SQLiteQuery;
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
		void copy_query(const char* new_query);

		/* Prepares the statement */
		void prepare_statement() throw(SQLiteError);

		SQLiteQuery(SQLite* parent, const char* query) throw(SQLiteError);
		virtual ~SQLiteQuery();
	public:	

		/* Executes the statement */
		void Execute(SQLiteResult** result=NULL) throw(SQLiteError);
		/* Resets the statement to use the given query */
		void Reset(const char* query) throw(SQLiteError);
		/* Binds values to the required parameters */
		void BindValue(const char* name, const SQLiteValue& value) throw(SQLiteError);
		void BindValue(int index, const SQLiteValue& value) throw(SQLiteError);
	
		friend class SQLite;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteValue
	// Purpose: Represents data for exchange between program and database.
	// Note: Values never get managed directly by SQLite and as such there
	// is no need to track the parent.
	class SQLiteValue : public SQLiteObject
	{
	private:
		VALUE_TYPES type;
				
		// stores a pointer to the data contained (could be of varying types)
		union 
		{
			int* i;
			double* d;
			std::string* s;
			std::wstring* ws;
			BYTE* b;
		} pdata;
		//void* pdata;
		size_t data_size;

		// Ensures the type of data stored is what's expected
		inline void VerifyType(int expected) const throw(SQLiteError) {
			if (expected != type) throw SQLiteError(SQL_ERRCODE_TYPE);
		}

		// Stop copying
		SQLiteValue& operator= (const SQLiteValue &v);
		SQLiteValue(const SQLiteValue &v);
	public:
		
		SQLiteValue(const char* val);
		SQLiteValue(const wchar_t* val);
		SQLiteValue(int val);
		SQLiteValue(double val);
		SQLiteValue(BYTE* val, size_t length);
		virtual ~SQLiteValue();

		/*	Get the row data in various data types, if the requested type
		 *	is not stored in this row a SQLiteError exception is thrown. Any
		 *	modifications made to pointed types is reflected in the internal
		 *	state. Do not free memory.
		 */
		std::string GetStr() const throw(SQLiteError);
		std::wstring GetWStr() const throw(SQLiteError);
		int GetInt() const throw(SQLiteError);
		double GetDouble() const throw(SQLiteError);
		BYTE* GetBlob() const throw(SQLiteError);

		/* Returns a string representation of the data held, non-string
		 * data is converted if necessary. */
		std::string ToString();

		int GetType() const { return type; }

		friend class SQLiteRow;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteRow
	// Purpose: Represent a row of data returned
	// Note: Rows never get managed directly by SQLite and as such there
	// is no need to track the parent.
	class SQLiteRow : public SQLiteObject
	{
	private:
		std::vector<SQLiteValue*> columns;

		/* Adds data to the row */
		void AddColumn(SQLiteValue* value);

		SQLiteRow();
		virtual ~SQLiteRow();
	public:
		
		SQLiteValue* operator [] (size_t i) throw(SQLiteError);
		SQLiteValue* get(size_t i) throw(SQLiteError);

		size_t size();

		friend class SQLiteQuery;
		friend class SQLiteResult;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteResult
	// Purpose: Represent a data set returned via SELECT statements
	class SQLiteResult : public SQLiteObject
	{
	private: 
		std::vector<SQLiteRow*> rows;
		
		void AddRow(SQLiteRow* row);

		SQLiteResult(SQLite* parent);
		virtual ~SQLiteResult();

	public:

		/* Returns the row at position i.
		 * Note: The value returned is simply a pointer to the row object
		 * and so any changes made to the row will be reflected in this class
		 * too.*/
		SQLiteRow* operator[] (size_t i) throw(SQLiteError);
		SQLiteRow* get(size_t i) throw(SQLiteError);

		size_t size();
		friend class SQLiteQuery;
	};
}