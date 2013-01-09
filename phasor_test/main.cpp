#include <stdio.h>
#include <boost/noncopyable.hpp>

class Base : boost::noncopyable
{
public:

};

class Derived : public Base
{

};
int main()
{
	Derived d;
	//Derived e = d;
}