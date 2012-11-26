#include <memory>

class Object
{

};

class DerivedObject : public Object
{

};

void SetGlobal(const char* name, const std::unique_ptr<Object>& object)
{
	
}

void RegisterFunction(const char* name)
{
	std::unique_ptr<DerivedObject> function(new DerivedObject());
	SetGlobal(name, function);
}


int main() {
	RegisterFunction("hello");
	return 0;
}