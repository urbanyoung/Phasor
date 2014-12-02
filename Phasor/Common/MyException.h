#pragma once

class CFailedCtor : public std::exception
{
public:
	CFailedCtor() : exception() {}
	CFailedCtor(const char* e) : exception(e) {}
	virtual ~CFailedCtor() {}
};