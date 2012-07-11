#include <string>
#include <stdio.h>

class A
{
public:
	virtual ~A(){
		printf("%s\n", __FUNCTION__);
	}

};

class B : public A
{
public:
	virtual ~B(){
		printf("%s\n", __FUNCTION__);
	}
};

class C : public B
{
public:
	C() {}
	C(int) {}
	virtual ~C()
	{
		printf("%s\n", __FUNCTION__);
	}
};

void test(C c, C d)
{

}

int main()
{
	C c;
	test(c, NULL);
	/*
	C* c = new C();
	A* cast = (A*)c;
	printf("Deleting casted value\n");
	delete cast;

	printf("Deleting typed value\n");
	C* c_ = new C();
	delete c_;
	*/

	return 0;
}