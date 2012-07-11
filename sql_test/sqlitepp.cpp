#include "sqlitepp.h"

namespace sqlite
{
	//-----------------------------------------------------------------------------------------
	// Class: SQLiteError
	//
	SQLiteError::SQLiteError(SQLPP_ERRCODE err) : std::exception() 
	{
		this->err = err;
		switch (err)
		{
		case SQL_ERRCODE_TYPE:
			{
				desc = "SQLiteTypeError - Attempted to access unexpected data type.";
			} break;
		}		
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
	
	void SQLite::free_object(SQLiteObject* obj)
	{
		std::set<SQLiteObject*>::iterator itr = allocated_objects.find(obj);
		if (itr != allocated_objects.end()) {
			allocated_objects.erase(itr);
			delete obj;
		}
	}
}