#pragma once
#pragma warning( disable : 4290 )

#include <windows.h>
#include <string>
#include <vector>
#include <map>

namespace Common
{
	class ObjectWrap;

	typedef std::shared_ptr<ObjectWrap> ObjectWrapPtr;

	/* Types of data that can be stored in ObjectWrap */
	enum VALUE_TYPES 
	{
		TYPE_NONE = -1,
		TYPE_STRING,
		TYPE_WSTRING,
		TYPE_INT,
		TYPE_DOUBLE,
		TYPE_BLOB,
		TYPE_PTR,
	};

	/* Error codes */
	enum OBJECT_ERRCODE
	{
		OBJECT_TYPE = 0 // access unexpected type
	};

	// --------------------------------------------------------------------
	// Classes
	// These classes are used for wrapping objects for transferring between
	// contexts
	class ObjectError : public std::exception
	{
	protected:
		int err;
		std::string msg;
		bool processed;

		bool IsProcessed() const { return processed; }

	public:
		ObjectError(int error, const char* desc=NULL);
		virtual ~ObjectError();
		virtual const char* what() const throw();
		const int type() { return err; }
	};
		
	//---------------------------------------------------------------------
	// Class: ObjectWrap
	// Purpose: Represent an object
	class ObjectWrap
	{
	private:	
		/* Store the data in a union for easy type casting */
		union {
			int* i;
			double* d;
			std::string* s;
			std::wstring* ws;
			BYTE* b;
			BYTE** ptr;
		} pdata;
		size_t length;
		int type;

		// Ensures the type of data stored is what's expected
		inline void VerifyType(int expected) const throw(ObjectError) {
			if (expected != type) throw ObjectError(OBJECT_TYPE);
		}
		
		void Copy(const ObjectWrap& v);

	public:
		ObjectWrap(const char* val);
		ObjectWrap(const wchar_t* val);
		ObjectWrap(int val);
		ObjectWrap(double val);
		ObjectWrap(BYTE* val, size_t length);
		ObjectWrap(void* ptr);
		ObjectWrap();
		virtual ~ObjectWrap();

		ObjectWrap& operator= (const ObjectWrap &v);
		ObjectWrap(const ObjectWrap &v);

		/*	Get the row data in various data types, if the requested type
		 *	is not stored in this row a ObjectError exception is thrown. Any
		 *	modifications made to pointed types is reflected in the internal
		 *	state. Do not free memory. */
		std::string GetStr() const throw(ObjectError);
		std::wstring GetWStr() const throw(ObjectError);
		int GetInt() const throw(ObjectError);
		double GetDouble() const throw(ObjectError);
		BYTE* GetBlob() const throw(ObjectError);
		void* GetPtr() const throw(ObjectError);

		/* Returns a string representation of the data held, non-string
		 * data is converted if necessary. */
		std::string ToString();

		/* Returns the type of the stored data */
		int GetType() const { return type; }

		/* Returns the size of the stored data in bytes (for blobs only).*/
		int size() const { return length; }
	};	

	// --------------------------------------------------------------------
	// Memory commands
	BOOL WriteBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);
	BOOL ReadBytes(DWORD dwAddress, LPVOID lpBuffer, DWORD dwCount);
	std::vector<DWORD> FindSignature(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpSignature, DWORD dwSignatureSize, LPBYTE lpWildCards = 0);
	DWORD FindAddress(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpSignature, DWORD dwSignatureSize, LPBYTE lpWildCards = 0, DWORD dwIndex = 0, DWORD dwOffset = 0);
	BOOL CreateCodeCave(DWORD dwAddress, BYTE cbSize, VOID (*pFunction)());

	// --------------------------------------------------------------------
	// String commands
	std::string NarrowString(std::wstring&);
	std::wstring WidenString(std::string&);

	// Format strings
	std::string FormatVarArgs(const char* _Format, va_list ArgList);
	std::wstring WFormatVarArgs(const wchar_t* _Format, va_list ArgList);
	std::string m_sprintf_s(const char* _Format, ...);
	std::wstring m_swprintf_s(const wchar_t* _Format, ...);
	
	// Tokenization functions
	std::vector<std::string> TokenizeCommand(const std::string& str);

	template <class T>
	std::vector<T> TokenizeString(const T& str, const T& delim)
	{
		// http://www.gamedev.net/community/forums/topic.asp?topic_id=381544#TokenizeString
		std::vector<T> tokens;
		size_t p0 = 0, p1 = T::npos;
		while (p0 != T::npos) {
			p1 = str.find_first_of(delim, p0);
			if(p1 != p0) {
				T token = str.substr(p0, p1 - p0);
				tokens.push_back(token);
			}
			p0 = str.find_first_not_of(delim, p1);
		}
		return tokens;
	}
	
	// --------------------------------------------------------------------
	//
	// Windows error stuff
	// Formats the return value of GetLastError into a string
	void GetLastErrorAsText(std::string& out);
}
