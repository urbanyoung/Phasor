#include "sqlitepp.h"

namespace sqlite
{
	//-----------------------------------------------------------------------------------------
	// Class: SQLiteError
	//
	SQLiteError::SQLiteError(SQLPP_ERRCODE err, const char* sql_desc) : std::exception() 
	{
		msg = new std::stringstream;
		this->err = err;
		switch (err)
		{
		case SQL_ERRCODE_TYPE:
			*msg <<  "SQLiteTypeError - Attempted to access unexpected data type.";
			break;
		}		

		if (sql_desc)
			*msg << "\nsqlite3 description: " << sql_desc;
	}

	SQLiteError::~SQLiteError()
	{
		delete msg;
	}

	const char* SQLiteError::what() const throw()
	{ 
		return msg->str().c_str(); 
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLite
	//
	SQLite::SQLite(std::string path_to_db)
	{

	}

	SQLite::~SQLite()
	{
		std::set<SQLiteObject*>::iterator itr = allocated_objects.begin();
		while (itr != allocated_objects.end()) {
			delete *itr;
			itr = allocated_objects.erase(itr);
		}
	}
	
	void SQLite::untrack_object(SQLiteObject* obj)
	{
		std::set<SQLiteObject*>::iterator itr = allocated_objects.find(obj);
		if (itr != allocated_objects.end())
			allocated_objects.erase(itr);

	}

	void SQLite::free_object(SQLiteObject* obj)
	{
		//untrack_object(obj);
		delete obj; // SQLiteObject calls untrack_object
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteObject
	// Purpose: Base class for objects returned to the user
	// 
	SQLiteObject::~SQLiteObject()
	{
		if (parent)
			parent->untrack_object(this);
	}


	//-----------------------------------------------------------------------------------------
	// Class: SQLiteQuery
	// Purpose: Provide an interface for executing prepared queries
	// 
	SQLiteQuery::SQLiteQuery(SQLite* parent, const char* query)
		throw(SQLiteError) 
		: SQLiteObject()		
	{
		SetParent(parent);
		this->query = NULL;
		copy_query(query);
		
		prepare_statement();	
	}

	SQLiteQuery::~SQLiteQuery()
	{
		delete[] query;
		if (stmt) sqlite3_finalize(stmt);	
	}

	void SQLiteQuery::copy_query(const char* query)
	{
		if (query) delete[] query;
		int len = strlen(query);
		this->query = new char[len + 1];
		if (len) memcpy(this->query, query, len);
		this->query[len] = 0;
	}

	void SQLiteQuery::prepare_statement()
		throw(SQLiteError)
	{
		// Get a statement handle, on failure abort.
		if (sqlite3_prepare_v2(parent->GetSQLite(), query, -1,
			&stmt, NULL) != SQLITE_OK) 
		{
			throw SQLiteError(SQL_ERRCODE_UNK, 
				sqlite3_errmsg(parent->GetSQLite()));
		}
	}

	void SQLiteQuery::Execute(SQLiteResult* result) 
		throw(SQLiteError)
	{		

	}

	void SQLiteQuery::Reset(const char* query)
		throw(SQLiteError)
	{
		copy_query(query);
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
	}

	void SQLiteQuery::BindValue(const char* name, SQLiteValue value) 
		throw(SQLiteError)
	{
		if (!stmt) throw SQLiteError(SQL_ERRCODE_NO_INIT);
		BindValue(sqlite3_bind_parameter_index(stmt, name), value);		
	}

	void SQLiteQuery::BindValue(int index, SQLiteValue value)
		throw(SQLiteError)
	{
		if (!stmt) throw SQLiteError(SQL_ERRCODE_NO_INIT);
		if (!index) throw SQLiteError(SQL_ERRCODE_NO_PARAM);

		int result = 0;
		switch (value.GetType())
		{
		case TYPE_STRING:
			result = sqlite3_bind_text(stmt, index, value.GetStr().c_str(),
				-1, SQLITE_TRANSIENT);
			break;
		case TYPE_WSTRING:
			result = sqlite3_bind_text16(stmt, index, value.GetWStr().c_str(),
					-1, SQLITE_TRANSIENT);
			break;
		case TYPE_INT:
			result = sqlite3_bind_int(stmt, index, value.GetInt());
			break;
		case TYPE_DOUBLE:
			result = sqlite3_bind_double(stmt, index, value.GetDouble());
			break;
		}

		// Handle any errors
		switch (result)
		{
		case SQLITE_RANGE:
			throw SQLiteError(SQL_ERRCODE_NO_PARAM);
			break;
		case SQLITE_MISUSE:
			throw SQLiteError(SQL_ERRCODE_MISUSE);
			break;
		}	
	}
}