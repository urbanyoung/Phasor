#pragma once
#pragma warning( disable : 4290 )

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <windows.h>
#include "../sqlite/sqlite3.h"

#pragma comment(lib, "../release/sqlite.lib")

namespace sqlite 
{
	class SQLiteObject;
	class SQLiteRow;
	class SQLiteResult;
	class SQLiteValue;
	class SQLiteQuery;
	class SQLite;

	typedef std::shared_ptr<SQLite> SQLitePtr; 
	typedef std::shared_ptr<SQLiteQuery> SQLiteQueryPtr; 
	typedef std::shared_ptr<SQLiteResult> SQLiteResultPtr;
	typedef std::shared_ptr<SQLiteRow> SQLiteRowPtr;
	typedef std::shared_ptr<SQLiteValue> SQLiteValuePtr;
	typedef std::shared_ptr<SQLiteValue> SQLiteObjectPtr;

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
		SQL_ERRCODE_CLOSED, // The database has been closed
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
	class SQLite : public std::enable_shared_from_this<SQLite>
	{
	private:
		sqlite3* sqlhandle;
		SQLite(const std::string& path_to_db) throw(SQLiteError);

		std::shared_ptr<SQLite> get_shared();

	public:
		static void Connect(SQLitePtr* ptr, const std::string& path_to_db) throw(SQLiteError);
		~SQLite();

		/* Closes the database connection */
		void Close();

		/* Creates a new query */
		void SQLite::NewQuery(SQLiteQueryPtr* ptr, const char* query);

		/* Returns a pointer to the open sqlite3 database. If the database
		 * has been closed an exception is thrown. */
		sqlite3* GetSQLite() throw (SQLiteError);

		friend class SQLiteQuery;
		friend class SQLiteObject;
	};

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteObject
	// Purpose: Base class for objects returned to the user
	class SQLiteObject
	{
	protected:
		SQLitePtr parent;

		/* Associates the object with a "parent" so it can perform sqlite
		 * operations*/
		void SetParent(SQLitePtr& parent);

	public:
		SQLiteObject();
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
		
	public:	

		SQLiteQuery(SQLitePtr& parent, const char* query) throw(SQLiteError);
		virtual ~SQLiteQuery();

		/* Executes the statement */
		void Execute(SQLiteResultPtr* out_result=NULL) throw(SQLiteError);
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
		/*
		 * This structure is used to hold the data for this value. It is
		 * managed by a shared pointer and as such making copies of the
		 * SQLiteValue object is safe. */
		struct c_data
		{
			/* Store the data in a union for easy type casting */
			union {
				int* i;
				double* d;
				std::string* s;
				std::wstring* ws;
				BYTE* b;
			} pdata;
			size_t size;
			int type;

			c_data(const char* val);
			c_data(const wchar_t* val);
			c_data(int val);
			c_data(double val);
			c_data(BYTE* val, size_t length);
			~c_data();
		};
		std::shared_ptr<c_data> data;

		// Ensures the type of data stored is what's expected
		inline void VerifyType(int expected) const throw(SQLiteError) {
			if (expected != data->type) throw SQLiteError(SQL_ERRCODE_TYPE);
		}
		
	public:
		SQLiteValue(const char* val);
		SQLiteValue(const wchar_t* val);
		SQLiteValue(int val);
		SQLiteValue(double val);
		SQLiteValue(BYTE* val, size_t length);
		virtual ~SQLiteValue();

		SQLiteValue& operator= (const SQLiteValue &v);
		SQLiteValue(const SQLiteValue &v);

		/*	Get the row data in various data types, if the requested type
		 *	is not stored in this row a SQLiteError exception is thrown. Any
		 *	modifications made to pointed types is reflected in the internal
		 *	state. Do not free memory. */
		std::string GetStr() const throw(SQLiteError);
		std::wstring GetWStr() const throw(SQLiteError);
		int GetInt() const throw(SQLiteError);
		double GetDouble() const throw(SQLiteError);
		BYTE* GetBlob() const throw(SQLiteError);

		/* Returns a string representation of the data held, non-string
		 * data is converted if necessary. */
		std::string ToString();

		/* Returns the type of the stored data */
		int GetType() const { return data->type; }

		/* Returns the size of the stored data in bytes. 
		 * Note: The size returned indicates two different things depending
		 * on the object type.
		 * 1) If the object is a blob the returned value indicates the
		 * size of the blob.
		 * 2) Otherwise the returned value indicates the size of the pointer
		 * to the stored data (ie 4 bytes). 
		 */
		int size() const { return data->size; }

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
		/* data stored in the column */
		std::vector<SQLiteValuePtr> columns;

		/* Adds data to the row */
		void AddColumn(SQLiteValue* value);
		
	public:
		
		SQLiteRow();
		virtual ~SQLiteRow();

		SQLiteValuePtr operator [] (size_t i) throw(SQLiteError);
		SQLiteValuePtr get(size_t i) throw(SQLiteError);

		/* Returns the number of items in the row*/
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
		/* used to store the retrieved rows */
		std::vector<SQLiteRowPtr> rows;
		
		/* Adds a new row to the result */
		void AddRow(SQLiteRow* row);
		
	public:

		SQLiteResult(SQLitePtr& parent);
		virtual ~SQLiteResult();

		/* Returns the row at position i.
		 * Note: The value returned is simply a pointer to the row object
		 * and so any changes made to the row will be reflected in this class
		 * too.*/
		SQLiteRowPtr operator[] (size_t i) throw(SQLiteError);
		SQLiteRowPtr get(size_t i) throw(SQLiteError);

		size_t size();
		friend class SQLiteQuery;
	};
}