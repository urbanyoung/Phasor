#include <vld.h>
#include <string>
#include <stdio.h>
#include "sqlitepp.h"

int main()
{
	using namespace sqlite;
	try
	{
		SQLite sql("test.sqlite");
		std::unique_ptr<SQLiteQuery> query =
			sql.NewQuery("CREATE TABLE IF NOT EXISTS admins("
			//"id INTEGER PRIMARY KEY,"
			"id int,"
			"username varchar(16),"
			"password char(32), b BLOB)");

		query->Execute();
		query->Reset("INSERT INTO admins (id, username, password, b) VALUES(:id, :username, :password, :b)");
		query->BindValue(":id", 1);
		query->BindValue(":username", "dfgiujf");
		query->BindValue(":password", "pass2");
		BYTE ptr[4096] = {0};
		query->BindValue(":b", SQLiteValue(ptr, sizeof(ptr)));
		query->Execute();

		query->Reset("SELECT * FROM admins");
		printf("Executing SELECT\n");
		std::unique_ptr<SQLiteResult> result = query->Execute();

		printf("Received %i rows of data\n", result->size());
		for (size_t x = 0; x < result->size(); x++) {
			SQLiteRow& row = result->get(x);
			Common::ObjWString& username = (Common::ObjWString&)row["username"];
			wprintf(L"username: %s\n", username.GetValue());
		}
	}
	catch (SQLiteError & e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}

/*class B
{
	int i;
public:
	B(int i)
	{
		this->i = i;
		printf("%s on %i\n", __FUNCTION__, i);
	}
	~B()
	{
		printf("%s on %i\n", __FUNCTION__, i);
	}
};

class A
{
private:
	union dtunion
	{
		int* i;
		double* d;
		std::string* s;
		std::wstring* ws;
		BYTE* b;
	} pdata;

	struct data
	{
		union dtunion
		{
			int* i;
			double* d;
			std::string* s;
			std::wstring* ws;
			BYTE* b;
		} pdata;
		int type;

		data::~data()
		{
			printf("deleting\n");
		}
	};


	std::shared_ptr<data> ptr;
	//A(const A &v);
public:
	A()
	{
		ptr.reset(new data);
	}

	A& operator= (const A &v)
	{
		printf("Assignment\n");
		ptr = v.ptr;
		return *this;
	}

	A(const A &v)
	{
		printf("Copy constructor\n");
		ptr = v.ptr;
	}

	~A()
	{
		//ptr->
		printf("Destructor\n");
		//ptr.reset();
	} 
};

void test1(const A& a)
{
	//printf("%08X\n", a);

}
void test(const A& a)
{
	test1(a);
}

void test2(A a)
{

}

int main()
{
	A a;
	A b = a;
	//a = b;
	return 0;
}*/