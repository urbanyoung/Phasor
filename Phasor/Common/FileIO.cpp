#include "FileIO.h"
#include <sstream>
#include <Shlobj.h>

namespace NDirectory
{
	// Attempts to create the specified simple directory
	bool CreateDirectory(const std::wstring& dir)
	{
		return ::CreateDirectoryW(dir.c_str(), NULL) == TRUE || 
			GetLastError() == ERROR_ALREADY_EXISTS;
	}

	// Ensure the directory ends with a single kDirSeparator character.
	void NormalizeDirectory(std::wstring& dir)
	{
		size_t dirEnd = dir.size();
		if (dirEnd == 0) return;
		wchar_t lastChar;

		do {
			dirEnd--;
			lastChar = dir.at(dirEnd);
		} while (dirEnd > 0 && lastChar == kDirSeparator);

		if (dirEnd == 0) {
			// no data or all slashes
			dir.clear();
			return;
		}

		if (dirEnd + 1 == dir.size())
			dir.push_back(kDirSeparator);
		else
			dir = dir.substr(0, dirEnd + 2);
	}

	// Get the file name from the given path -- excluding extension.
	void GetFileName(const std::wstring& path, std::wstring& fileName)
	{
		size_t fileStart = path.find_last_of(kDirSeparator);
		if (fileStart == path.npos) {
			fileStart = 0;
		}
		// check for extension
		size_t fileEnd = path.size(); // assume no extension
		size_t extensionStart = path.find_last_of(kExtensionSeparator);
		if (extensionStart != path.npos) fileEnd = extensionStart;
		fileName = path.substr(fileStart, fileEnd-fileStart);
	}

	// Strip the file name, and extension, from the given path (if there is one)
	void StripFile(std::wstring& path)
	{
		size_t fileStart = path.find_last_of(kDirSeparator);
		if (fileStart == path.npos) return;
		path = path.substr(0, fileStart);
	}

	bool IsDirectory(const std::wstring& path)
	{
		DWORD dwAttributes = GetFileAttributesW(path.c_str());

		return dwAttributes != INVALID_FILE_ATTRIBUTES && 
			dwAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}

	// Checks if the specified file exists
	bool IsValidFile(const std::wstring& path)
	{
		DWORD dwAttributes = GetFileAttributesW(path.c_str());
		return dwAttributes != INVALID_FILE_ATTRIBUTES && 
			!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	// Gets the files (not any directories) within a directory matching
	// the specified pattern
	void FindFiles(const std::wstring& searchExp, 
		std::list<std::wstring>& files)
	{
		WIN32_FIND_DATAW data;
		HANDLE hFind = FindFirstFileW(searchExp.c_str(), &data);
		if (hFind == INVALID_HANDLE_VALUE) return;

		try {
			do {
				// Make sure it's a file
				if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					files.push_back(data.cFileName);
				}
			} while (FindNextFileW(hFind, &data) != 0);
		}
		catch (...)
		{
			FindClose(hFind);
			throw;
		}
		FindClose(hFind);
	}

#ifdef _WIN32
	bool GetMyDocuments(std::wstring& path)
	{
		wchar_t szOut[MAX_PATH] = {0};
		if (SHGetFolderPathW(0, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, szOut) != S_OK)
			return false;
		path = szOut;
		return true;
	}
#endif
}

CFile::CFile() : hFile(INVALID_HANDLE_VALUE)
{
}

CFile::~CFile()
{
	Close();
}

bool CFile::Open(const std::wstring& file, DWORD dwAccess, DWORD dwShared, DWORD dwCreateDeposition)
{
	HANDLE newFile = CreateFileW(file.c_str(), dwAccess, dwShared, NULL, dwCreateDeposition, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (newFile == INVALID_HANDLE_VALUE) return false;
	Close();
	hFile = newFile;
	file_name = file;
	return true;
}

bool CFile::Open(const std::wstring& file, DWORD dwAccess)
{
	return Open(file, dwAccess, FILE_SHARE_READ, OPEN_ALWAYS);
}

void CFile::Close()
{
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
}

bool CFile::IsOpen() const 
{ 
	return hFile != INVALID_HANDLE_VALUE; 
}

bool CFile::Move(const std::wstring& file,
	const std::wstring& newfile, bool overwrite)
{
	return MoveFileExW(file.c_str(), newfile.c_str(), 
		overwrite ? MOVEFILE_REPLACE_EXISTING : NULL) == TRUE;
}

bool CFile::Delete(const std::wstring& file)
{
	return DeleteFileW(file.c_str()) == TRUE;
}

bool CFile::SeekBegin()
{
	if (!IsOpen()) return false;
	return SetFilePointer(hFile, 0,0,FILE_BEGIN) == INVALID_SET_FILE_POINTER;
}

bool CFile::SeekEnd()
{
	if (!IsOpen()) return false;
	return SetFilePointer(hFile, 0,0,FILE_END) == INVALID_SET_FILE_POINTER;
}

DWORD CFile::GetFileSize()
{
	DWORD size = ::GetFileSize(hFile, NULL);
	return size == INVALID_FILE_SIZE ? 0 : size;
}

bool CFile::Seek(long distance)
{
	if (!IsOpen()) return false;
	return SetFilePointer(hFile, distance,0,FILE_CURRENT) == INVALID_SET_FILE_POINTER;
}

// ------------------------------------------------------------------------
//

CInFile::CInFile()
{
}

CInFile::~CInFile()
{
}

bool CInFile::Open(const std::wstring& file)
{
	return CFile::Open(file, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
}

bool CInFile::ReadSome(BYTE* data, DWORD size, DWORD* processedSize)
{
	if (!IsOpen()) return false;
	DWORD to_read = size > kReadSize ? kReadSize : size;
	return ReadFile(hFile, data, to_read, processedSize, NULL) == TRUE;
}

bool CInFile::Read(BYTE* data, DWORD size, DWORD* processedSize)
{
	*processedSize = 0;
	while (size > 0)
	{
		DWORD read = 0;
		bool result = ReadSome(data, size, &read);
		if (read == 0) return false; // could be eof
		size -= read;
		*processedSize += read;
		data += read;
		if (!result) return false;
	}
	return true;
}

// ------------------------------------------------------------------------
//
COutFile::COutFile()
{
}

COutFile::~COutFile()
{
}

bool COutFile::WriteSome(BYTE* data, DWORD size, DWORD* processedSize)
{
	if (!IsOpen()) return false;
	DWORD to_write = size > kWriteSize ? kWriteSize : size;
	return WriteFile(hFile, data, to_write, processedSize, NULL) == TRUE;
}

bool COutFile::Write(BYTE* data, DWORD size, DWORD* processedSize)
{
	*processedSize = 0;
	while (size > 0)
	{
		DWORD written = 0;
		if (!WriteSome(data, size, &written))
			return false;
		size -= written;
		*processedSize += written;
		data += written;
	}
	return true;
}