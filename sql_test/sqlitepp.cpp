#include "sqlitepp.h"
#include "../Phasor/Common.h"
namespace sqlite
{
	//-----------------------------------------------------------------------------------------
	// Class: SQLiteError
	//
	SQLiteError::SQLiteError(int err, const char* sql_desc) : std::exception() 
	{
		std::stringstream s;
		this->err = err;
		switch (err)
		{
		case SQL_ERRCODE_TYPE:
			s <<  "SQLiteTypeError - Attempted to access unexpected data type.";
			break;

		case SQL_ERRCODE_NDE:
			s << "SQLiteError - Executed query where no data was expected but received data.";
			break;
		default:
			s << "SQLiteError - Unknown code: " << err;
		}		

		if (sql_desc)
			s << "\ndescription: " << std::string(sql_desc);
		msg.assign(s.str());
	}

	SQLiteError::~SQLiteError()
	{
	}

	const char* SQLiteError::what() const throw()
	{ 
		return msg.c_str();
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLite
	//
	SQLite::SQLite(std::string path_to_db) throw(SQLiteError)
	{
		int result = sqlite3_open(path_to_db.c_str(), &sqlhandle);

		if (result != SQLITE_OK) // some error
		{
			std::string desc = sqlite3_errmsg(sqlhandle);
			sqlite3_close(sqlhandle);
			throw SQLiteError(SQL_ERRCODE_UNK, desc.c_str());
		}
		alive = true;
	}

	SQLite::~SQLite()
	{
		alive = false; // used to stop untrack_object from changing allocated_objects

		std::set<SQLiteObject*>::iterator itr = allocated_objects.begin();
		while (itr != allocated_objects.end()) {
			delete *itr;
			itr = allocated_objects.erase(itr);
		}
		allocated_objects.clear();
		sqlite3_close(sqlhandle);
	}
	
	void SQLite::untrack_object(SQLiteObject* obj)
	{
		if (!alive) return;
		std::set<SQLiteObject*>::iterator itr = allocated_objects.find(obj);
		if (itr != allocated_objects.end())
			allocated_objects.erase(itr);
	}

	void SQLite::track_object(SQLiteObject* obj)
	{
		allocated_objects.insert(obj);
	}

	void SQLite::free_object(SQLiteObject* obj)
	{
		//untrack_object(obj);
		delete obj; // SQLiteObject calls untrack_object
	}

	SQLiteQuery* SQLite::NewQuery(const char* query)
	{
		SQLiteQuery* q = new SQLiteQuery(this, query);
		track_object(q);
		return q;
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteObject
	// Purpose: Base class for objects returned to the user
	// 
	SQLiteObject::~SQLiteObject()
	{
		if (parent)	parent->untrack_object(this);
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

	void SQLiteQuery::copy_query(const char* new_query)
	{
		if (this->query) delete[] this->query;
		int len = strlen(new_query);
		this->query = new char[len + 1];
		if (len) memcpy(this->query, new_query, len);
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

	void SQLiteQuery::Execute(SQLiteResult** out_result) 
		throw(SQLiteError)
	{		
		SQLiteResult* tmp_result = new SQLiteResult(parent);

		int error = SQL_ERR_NONE;
		while (1)
		{
			int result = sqlite3_step(stmt);
			if (result == SQLITE_DONE) break;

			if (result == SQLITE_ROW) { // data!
				if (!out_result) { // no data was expected
					error = SQL_ERRCODE_NDE;
					continue;
					// break; continue with the query and throw at end
				}
				SQLiteRow* row = new SQLiteRow();

				int columns = sqlite3_column_count(stmt);
				for (int x = 0; x < columns; x++) {
					int type = sqlite3_column_type(stmt, x);
					switch (type)
					{
					case SQLITE_INTEGER:
						row->AddColumn(
							new SQLiteValue(sqlite3_column_int(stmt, x)));
						break;
					case SQLITE_FLOAT:
						row->AddColumn(
							new SQLiteValue(sqlite3_column_double(stmt, x)));
						break;
					case SQLITE_TEXT:
						//printf("Normal: %i, 16: %i\n", 
						//	sqlite3_column_bytes(stmt, x), sqlite3_column_bytes16(stmt, x));
						// need to convert char* to widechar and assume
						// all received text is wide
						row->AddColumn(
							new SQLiteValue((const wchar_t*)sqlite3_column_text16(stmt, x)));
					}
				}
				tmp_result->AddRow(row);
			}
			else { // error
				delete tmp_result;
				throw SQLiteError(SQL_ERRCODE_UNK, 
				sqlite3_errmsg(parent->GetSQLite()));
			}
		}

		// If there's been an error throw it
		if (error != SQL_ERR_NONE) {
			delete tmp_result;
			throw SQLiteError(error ,query);
		}

		if (out_result) {
			parent->track_object(tmp_result);
			*out_result = tmp_result;
		} else 
			delete tmp_result;
	}

	void SQLiteQuery::Reset(const char* query)
		throw(SQLiteError)
	{
		copy_query(query);
		sqlite3_finalize(stmt);
		prepare_statement();
	}

	void SQLiteQuery::BindValue(const char* name, const SQLiteValue& value) 
		throw(SQLiteError)
	{
		if (!stmt) throw SQLiteError(SQL_ERRCODE_NO_INIT);
		BindValue(sqlite3_bind_parameter_index(stmt, name), value);	
	}

	void SQLiteQuery::BindValue(int index, const SQLiteValue& value)
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

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteValue
	// 
	SQLiteValue::SQLiteValue(const char* val) : SQLiteObject()
	{
		pdata.s = new std::string;
		pdata.s->assign(val);
		type = TYPE_STRING;
		//init((void*)str, 0); // 0 for now
	}

	SQLiteValue::SQLiteValue(const wchar_t* val) : SQLiteObject()
	{
		pdata.ws = new std::wstring;
		pdata.ws->assign(val);
		type = TYPE_WSTRING;
	}

	SQLiteValue::SQLiteValue(int val) : SQLiteObject()
	{
		pdata.i = new int;
		*pdata.i = val;
		type = TYPE_INT;
		//init((void*)i, 0);
	}

	SQLiteValue::SQLiteValue(double val) : SQLiteObject()
	{
		pdata.d = new double;
		*pdata.d = val;
		type = TYPE_DOUBLE;
		//init((void*)d, 0);
	}

	SQLiteValue::SQLiteValue(BYTE* val, size_t length) : SQLiteObject()
	{
		pdata.b = new BYTE[length];
		memcpy(pdata.b, val, length);
		type = TYPE_BLOB;
		//init((void*)p, length);
	}

	SQLiteValue::~SQLiteValue()
	{
		printf("Destroy type %i\n", type);
		//printf("Destroy: pdata: %08X\n", pdata);
		//printf("Destroying %08X Type: %i\n", this, type);
		switch (type)
		{
		case TYPE_INT:
			delete pdata.i;
			break;		
		case TYPE_STRING:
			delete pdata.s;
			break;
		case TYPE_WSTRING:
			delete pdata.ws;
			break;
		case TYPE_DOUBLE:
			delete pdata.d;
			break;
		case TYPE_BLOB:
			delete[] pdata.b;
			break;
		} 
	}
	
	std::string SQLiteValue::GetStr() const throw(SQLiteError)
	{
		// let strings convert themselves
		if (type == TYPE_WSTRING) return Common::NarrowString(*pdata.ws);
		VerifyType(TYPE_STRING);
		return *pdata.s;
	}

	std::wstring SQLiteValue::GetWStr() const throw(SQLiteError)
	{
		if (type == TYPE_STRING) return Common::WidenString(*pdata.s);
		VerifyType(TYPE_WSTRING);
		return *pdata.ws;
	}

	int SQLiteValue::GetInt() const throw(SQLiteError)
	{
		VerifyType(TYPE_INT);
		return *pdata.i;
	}

	double SQLiteValue::GetDouble() const throw(SQLiteError)
	{
		VerifyType(TYPE_DOUBLE);
		return *pdata.d;
	}

	std::string SQLiteValue::ToString()
	{
		std::stringstream s;
		switch (type)
		{
		case TYPE_STRING:
			s << *pdata.s;
			break;
		case TYPE_WSTRING:
			s << Common::NarrowString(*pdata.ws);
			break;
		case TYPE_INT:
			s << *pdata.i;
			break;
		case TYPE_DOUBLE:
			s << *pdata.d;
			break;
		case TYPE_BLOB:
			s << *pdata.i << " byte BLOB@" << *pdata.b;
			break;
		}
		std::string str = s.str();
		return str;
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteRow
	//
	SQLiteRow::SQLiteRow() : SQLiteObject()
	{

	}

	SQLiteRow::~SQLiteRow()
	{
		std::vector<SQLiteValue*>::iterator itr = columns.begin();
		while (itr != columns.end()){
			delete *itr; // cleanup value
			itr = columns.erase(itr);
		}
	}

	void SQLiteRow::AddColumn(SQLiteValue* value)
	{
		columns.push_back(value);
	}

	SQLiteValue* SQLiteRow::operator[] (size_t i) throw(SQLiteError)
	{
		return get(i);
	}

	SQLiteValue* SQLiteRow::get(size_t i) throw(SQLiteError)
	{
		if (i < 0 || i >= columns.size())
			throw SQLiteError(SQL_ERRCODE_BAD_INDEX);
		return columns[i];
	}

	size_t SQLiteRow::size() 
	{
		return columns.size();
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteResult
	//
	SQLiteResult::SQLiteResult(SQLite* parent) : SQLiteObject()
	{
		SetParent(parent);
	}

	SQLiteResult::~SQLiteResult()
	{
		std::vector<SQLiteRow*>::iterator itr = rows.begin();
		while (itr != rows.end()){
			delete *itr; // cleanup row
			itr = rows.erase(itr);
		}
	}

	void SQLiteResult::AddRow(SQLiteRow* row)
	{
		rows.push_back(row);
	}

	SQLiteRow* SQLiteResult::operator[] (size_t i) throw(SQLiteError)
	{
		return get(i);
	}

	SQLiteRow* SQLiteResult::get(size_t i) throw(SQLiteError)
	{
		if (i < 0 || i >= rows.size())
			throw SQLiteError(SQL_ERRCODE_BAD_INDEX);
		return rows[i];
	}

	size_t SQLiteResult::size()
	{
		return rows.size();
	}
}