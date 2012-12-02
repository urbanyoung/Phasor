#pragma once

#include <windows.h>
#include <string>

namespace NDirectory
{
#ifdef _WIN32
	const wchar_t kDirSeparator = L'\\';
#else
	const wchar_t kDirSeparator = L'/';
#endif
	const wchar_t kExtensionSeparator = L'.';

	// Attempts to create the specified simple directory
	bool CreateDirectory(const std::wstring& dir);

	// Ensure the directory ends with a single kDirSeparator character.
	void NormalizeDirectory(std::wstring& dir);

	// Get the file name from the given path -- excluding extension.
	void GetFileName(const std::wstring& path, std::wstring& fileName);

	// Strip the file name, and extension, from the given path
	void StripFile(const std::wstring& path);

	// Gets if the specified path points to a valid directory
	bool IsDirectory(const std::wstring& path);

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

	bool Seek(DWORD distance);
	bool SeekBegin();
	bool SeekEnd();	
};

class CInFile : public CFile
{

};

class COutFile : public CFile
{
private:
	static const int kWriteSize = 1 << 15; // 32kb

public:
	COutFile();
	virtual ~COutFile();

	// Attempts to write all data until either success or an error
	bool Write(BYTE* data, DWORD size, DWORD* processedSize);

	// Attempts to write data in 32kb blocks
	// False indicated an error, otherwise keep calling.
	bool WriteSome(BYTE* data, DWORD size, DWORD* written);
};