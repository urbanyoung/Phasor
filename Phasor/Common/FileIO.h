#pragma once

#include <windows.h>
#include <string>
#include <list>

namespace NDirectory
{
#ifdef _WIN32
	const wchar_t kDirSeparator = L'\\';
#else
	const wchar_t kDirSeparator = L'/';
#endif
	const wchar_t kExtensionSeparator = L'.';

	// Attempts to create the specified simple directory
	// ie its immediate parent must exist
	bool CreateDirectory(const std::wstring& dir);

	// Ensure the directory ends with a single kDirSeparator character.
	void NormalizeDirectory(std::wstring& dir);

	// Get the file name from the given path -- excluding extension.
	void GetFileName(const std::wstring& path, std::wstring& fileName);

	// Strip the file name, and extension, from the given path
	void StripFile(const std::wstring& path);

	// Gets if the specified path points to a valid directory
	bool IsDirectory(const std::wstring& path);

	// Checks if the specified file exists
	bool IsValidFile(const std::wstring& path);

	// Gets the files (not any directories) within a directory matching
	// the specified pattern
	void FindFiles(const std::wstring& searchExp, 
		std::list<std::wstring>& files);	

#ifdef _WIN32
	bool GetMyDocuments(std::wstring& path);
#endif
}

class CFile
{
protected:
	HANDLE hFile;
	std::wstring file_name;

public:
	CFile();
	~CFile();

	bool Open(const std::wstring& file, DWORD dwAccess, DWORD dwShared, DWORD dwCreateDeposition);
	bool Open(const std::wstring& file, DWORD dwAccess);
	bool IsOpen() const;

	void Close();

	// Move the specified file to the specified location.
	static bool Move(const std::wstring& file,
					const std::wstring& newfile, bool overwrite);

	// Delete the specified file
	static bool Delete(const std::wstring& file);

	DWORD GetFileSize();

	bool Seek(long distance);
	bool SeekBegin();
	bool SeekEnd();	
};

class CInFile : public CFile
{
private:
	static const int kReadSize = 1 << 20; // 1 MB
public:
	CInFile();
	virtual ~CInFile();

	bool Open(const std::wstring& file);
	
	// Reads data until a line escape(\n or \r\n) or until maxCount - 1 is 
	// reached, which is the maximum number of characters (of size sizeof(T)) 
	// to be read. The output buffer should be at least sizeof(T)*maxCount
	// bytes. The output is always null terminated. found is set to true if
	// newline found, or false otherwise.
	template <class T>
	bool ReadLine(T* out, DWORD maxCount, bool* found)
	{
		DWORD read;
		if (!ReadSome((BYTE*)out, sizeof(T)*(maxCount-1), &read)) return false;
		if (read == 0) return false; // maybe eof
		DWORD nCharRead = read / sizeof(T);
		size_t x = 0, end = 0;
		if (found) *found = false;
		for (; x < nCharRead; x++)
		{
			if (out[x] == T('\n')) {
				if (found) *found = true;
				break;
			}
			else if (out[x] == T('\r')) end = x;
		}
		if (end + 1 == x) out[end] = T('\0');
		else out[x] = T('\0');
		// seek to start of the next line
		if (read != x) {
			long distance = -(long)read + (x + 1);
			if (distance != 0) Seek(distance);
		}
		return true;
	}

	// Attempts to read all data until either success or an error
	bool Read(BYTE* out, DWORD size, DWORD* processedSize);

	// Attempts to read data in 1MB blocks
	// False indicated an error, otherwise keep calling.
	bool ReadSome(BYTE* out, DWORD size, DWORD* written);
};

class COutFile : public CFile
{
private:
	static const int kWriteSize = 1 << 20; // 1 MB

public:
	COutFile();
	virtual ~COutFile();

	// Attempts to write all data until either success or an error
	bool Write(BYTE* data, DWORD size, DWORD* processedSize);

	// Attempts to write data in 1MB blocks
	// False indicated an error, otherwise keep calling.
	bool WriteSome(BYTE* data, DWORD size, DWORD* written);
};

template <class T>
class CTempFile : public T
{
private:
	bool do_delete;

public:
	CTempFile() : T(), do_delete(true) {}
	virtual ~CTempFile() {
		if (do_delete) {
			Close();
			CFile::Delete(file_name);
		}
	}

	void success() { do_delete = false; }
};