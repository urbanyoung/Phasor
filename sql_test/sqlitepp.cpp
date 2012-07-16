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
		case SQL_ERRCODE_FAILED_OPEN:
			s << "SQLiteError - Unable the open the database file.";
			break;
		case SQL_ERRCODE_TYPE:
			s <<  "SQLiteError - Attempted to access unexpected data type.";
			break;
		case SQL_ERRCODE_NO_INIT:
			s << "SQLiteError - Attempted to use uninitialized data.";
			break;
		case SQL_ERRCODE_NO_PARAM:
			s << "SQLiteError - Attempted to bind to an unknown parameter";
			break;
		case SQL_ERRCODE_BAD_INDEX:
			s << "SQLiteError - Out of bounds access attempt on SQLiteResult or SQLiteRow";
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
	SQLite::SQLite(const std::string& path_to_db) throw(SQLiteError)
	{
		int result = sqlite3_open(path_to_db.c_str(), &sqlhandle);

		if (result != SQLITE_OK) // some error
		{
			std::string desc = sqlite3_errmsg(sqlhandle);
			sqlite3_close(sqlhandle);
			throw SQLiteError(SQL_ERRCODE_UNK, desc.c_str());
		}
	}

	SQLite::~SQLite()
	{
		printf("%s\n", __FUNCTION__);
		Close();
	}

	void SQLite::Connect(SQLitePtr* ptr, const std::string& path_to_db) throw(SQLiteError)
	{
		*ptr = SQLitePtr(new SQLite(path_to_db));
	}

	std::shared_ptr<SQLite> SQLite::get_shared()
	{
		return shared_from_this();
	}
	
	void SQLite::Close()
	{
		if (sqlhandle) {
			sqlite3_close(sqlhandle);
			sqlhandle = NULL;
		}
	}

	sqlite3* SQLite::GetSQLite() throw (SQLiteError)
	{
		if (!sqlhandle) throw SQLiteError(SQL_ERRCODE_CLOSED);
		return sqlhandle;
	}

	void SQLite::NewQuery(SQLiteQueryPtr* ptr, const char* query)
	{
		SQLiteQuery* q = new SQLiteQuery(get_shared(), query);
		ptr->reset(q);
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteObject
	// Purpose: Base class for objects returned to the user
	// 
	SQLiteObject::SQLiteObject() : parent(NULL)
	{

	}

	void SQLiteObject::SetParent(SQLitePtr& parent)
	{
		//printf("Setting parent: %08X\n", parent);
		this->parent = parent;
	}

	SQLiteObject::~SQLiteObject()
	{
		//printf("Destroyed\n");
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteQuery
	// Purpose: Provide an interface for executing prepared queries
	// 
	SQLiteQuery::SQLiteQuery(SQLitePtr& parent, const char* query)
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

	void SQLiteQuery::Execute(SQLiteResultPtr* out_result) 
		throw(SQLiteError)
	{		
		if (!stmt) throw SQLiteError(SQL_ERRCODE_NO_INIT);
		SQLiteResult* tmp_result = new SQLiteResult(parent);
		char* err_info = query;

		int error = SQL_ERR_NONE;
		while (1)
		{
			int result = sqlite3_step(stmt);
			if (result == SQLITE_DONE) break;

			if (result == SQLITE_ROW) { // data!
				if (!out_result) { // no data was expected
					error = SQL_ERRCODE_NDE;
					break;
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
						// all text is treated as wide and converted if necessary
						row->AddColumn(
							new SQLiteValue((const wchar_t*)sqlite3_column_text16(stmt, x)));
						break;
					case SQLITE_BLOB:
						row->AddColumn(new SQLiteValue((BYTE*)sqlite3_column_blob(stmt, x),
							sqlite3_column_bytes(stmt, x)));
						break;
					}
				}
				tmp_result->AddRow(row);
			}
			else { // error
				err_info = (char*)sqlite3_errmsg(parent->GetSQLite());
				error = SQL_ERRCODE_UNK;
				break;
			}
		}

		sqlite3_finalize(stmt);
		stmt = NULL;
		// If there's been an error throw it
		if (error != SQL_ERR_NONE) {
			delete tmp_result;
			throw SQLiteError(error,err_info);
		}

		if (out_result) {
			//parent->track_object(tmp_result);
			out_result->reset(tmp_result);
		} else 
			delete tmp_result;
	}

	void SQLiteQuery::Reset(const char* query)
		throw(SQLiteError)
	{
		copy_query(query);
		if (stmt) {
			sqlite3_finalize(stmt);
			stmt = NULL;
		}
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
		case TYPE_BLOB:
			result = sqlite3_bind_blob(stmt, index, value.GetBlob(), 
				value.size(), SQLITE_TRANSIENT);
			break;
		}

		// Handle any errors
		switch (result)
		{
		case SQLITE_RANGE:
			throw SQLiteError(SQL_ERRCODE_NO_PARAM, sqlite3_errmsg(parent->GetSQLite()));
			break;
		case SQLITE_MISUSE:
			throw SQLiteError(SQL_ERRCODE_MISUSE, sqlite3_errmsg(parent->GetSQLite()));
			break;
		case SQLITE_OK:
			break;
		default:
			throw SQLiteError(SQL_ERRCODE_UNK, sqlite3_errmsg(parent->GetSQLite()));
			break;
		}	
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteValue
	// 
	int created = 0;
	SQLiteValue::c_data::c_data(const char* val) : size(4) {
		created++;
		pdata.s = new std::string;
		pdata.s->assign(val);
		type = TYPE_STRING;
	}
	SQLiteValue::c_data::c_data(const wchar_t* val) : size(4) {
		created++;
		pdata.ws = new std::wstring;
		pdata.ws->assign(val);
		type = TYPE_WSTRING;
	}
	SQLiteValue::c_data::c_data(int val) : size(4) {
		created++;
		pdata.i = new int;
		*pdata.i = val;
		type = TYPE_INT;
	}
	SQLiteValue::c_data::c_data(double val) : size(4) {
		created++;
		pdata.d = new double;
		*pdata.d = val;
		type = TYPE_DOUBLE;			
	}
	SQLiteValue::c_data::c_data(BYTE* val, size_t length) : size(length) { 
		created++;
		pdata.b = new BYTE[length];
		memcpy(pdata.b, val, length);
		type = TYPE_BLOB;
	}

	SQLiteValue::c_data::~c_data()
	{
		created--;
		printf("%i open objects\n", created);
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

	SQLiteValue::SQLiteValue(const char* val) : data(new c_data(val))
	{		
	}

	SQLiteValue::SQLiteValue(const wchar_t* val) : data(new c_data(val))
	{		
	}

	SQLiteValue::SQLiteValue(int val) : data(new c_data(val))
	{		
	}

	SQLiteValue::SQLiteValue(double val) : data(new c_data(val))
	{		
	}

	SQLiteValue::SQLiteValue(BYTE* val, size_t length) : data(new c_data(val, length))
	{		
	}

	SQLiteValue::SQLiteValue(const SQLiteValue &v) : data(v.data)
	{		
	}

	SQLiteValue& SQLiteValue::operator= (const SQLiteValue &v)
	{
		//printf("%08X %08X\n", v, this);
		data = v.data;
		return *this;
	}

	SQLiteValue::~SQLiteValue()
	{
		//printf("Destruct type %i\n", data->type);
	}
	
	std::string SQLiteValue::GetStr() const throw(SQLiteError)
	{
		// let strings convert themselves
		if (data->type == TYPE_WSTRING) return Common::NarrowString(*data->pdata.ws);
		VerifyType(TYPE_STRING);
		return *(data->pdata.s);
	}

	std::wstring SQLiteValue::GetWStr() const throw(SQLiteError)
	{
		if (data->type == TYPE_STRING) return Common::WidenString(*data->pdata.s);
		VerifyType(TYPE_WSTRING);
		return *(data->pdata.ws);
	}

	int SQLiteValue::GetInt() const throw(SQLiteError)
	{
		VerifyType(TYPE_INT);
		return *(data->pdata.i);
	}

	double SQLiteValue::GetDouble() const throw(SQLiteError)
	{
		VerifyType(TYPE_DOUBLE);
		return *(data->pdata.d);
	}

	BYTE* SQLiteValue::GetBlob() const throw(SQLiteError)
	{
		VerifyType(TYPE_BLOB);
		return data->pdata.b;
	}

	std::string SQLiteValue::ToString()
	{
		std::stringstream s;
		switch (data->type)
		{
		case TYPE_STRING:
			s << *data->pdata.s;
			break;
		case TYPE_WSTRING:
			s << Common::NarrowString(*data->pdata.ws);
			break;
		case TYPE_INT:
			s << *data->pdata.i;
			break;
		case TYPE_DOUBLE:
			s << *data->pdata.d;
			break;
		case TYPE_BLOB:
			s << data->size << " byte BLOB@" << (DWORD)data->pdata.b;
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
		columns.clear();
	}

	void SQLiteRow::AddColumn(SQLiteValue* value)
	{
		columns.push_back(SQLiteValuePtr(value));
	}

	SQLiteValuePtr SQLiteRow::operator[] (size_t i) throw(SQLiteError)
	{
		return get(i);
	}

	SQLiteValuePtr SQLiteRow::get(size_t i) throw(SQLiteError)
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
	SQLiteResult::SQLiteResult(SQLitePtr& parent) : SQLiteObject()
	{
		SetParent(parent);
	}

	SQLiteResult::~SQLiteResult()
	{
		rows.clear(); // smart pointers do cleanup
	}

	void SQLiteResult::AddRow(SQLiteRow* row)
	{
		rows.push_back(SQLiteRowPtr(row));
	}

	SQLiteRowPtr SQLiteResult::operator[] (size_t i) throw(SQLiteError)
	{
		return get(i);
	}

	SQLiteRowPtr SQLiteResult::get(size_t i) throw(SQLiteError)
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