#pragma once

class noncopyable
{
	noncopyable(const noncopyable& other);
	noncopyable& operator= (const noncopyable& rhs);
public:
	noncopyable(){}
};