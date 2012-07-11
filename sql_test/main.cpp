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
	virtual ~C()
	{
		printf("%s\n", __FUNCTION__);
	}

};
int main()
{
	C* c = new C();
	A* cast = (A*)c;
	delete cast;

	C* c_ = new C();
	delete c_;
	

	return 0;
}