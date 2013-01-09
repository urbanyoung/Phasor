#pragma once

#include "../Common/Streams.h"
#include <memory>

// Stream which output data to a file with .log extension and timestamps
// each entry.
class CLoggingStream : public COutStream
{
protected:
	std::wstring moveDirectory; // directory to move file to
	std::wstring filePath; // path to file inclusive of name and extension
	std::wstring fileName; //  name of file (no path info, no extension)
	std::wstring fileDirectory;

	DWORD byteSize; // max size for file before being moved
	DWORD errorOffset; // increases when moving the file fails

	bool bTimestamp; // should timestamps be prepended? default true
private:
	void Initialize(const std::wstring& directory,
		const std::wstring& fileName, const std::wstring& moveDirectory);
	void SetNames(const std::wstring& directory,
		const std::wstring& fileName, const std::wstring& moveDirectory);

	// Check if the file should be moved.
	void CheckAndMove(DWORD curSize);
	CLoggingStream& operator=(const CLoggingStream& rhs);

protected:
	virtual bool Write(const std::wstring& str) override;
	virtual std::unique_ptr<COutStream> clone() override
	{
		return std::unique_ptr<COutStream>(new CLoggingStream(*this));
	}

	CLoggingStream(const CLoggingStream& other);

public:
	// directory should be normalized (finish with a single \\)
	CLoggingStream(const std::wstring& fileDirectory, 
		const std::wstring& fileName, const std::wstring& moveDirectory);
	
	virtual ~CLoggingStream();

	virtual void SetMoveSize(DWORD kbSize);
	virtual void SetMoveDirectory(const std::wstring& move_to);
	virtual void SetOutFile(const std::wstring& directory,const std::wstring& fileName);
	virtual void SetOutFile(const std::wstring& fileName); // use cur dir
	virtual void EnableTimestamp(bool state); // true by default
	virtual bool DoTimestamp() { return bTimestamp; }
	static std::wstring PrependTimestamp(const std::wstring& str);
};

typedef CLoggingStream CPhasorLog;
typedef CLoggingStream CScriptsLog;
typedef CLoggingStream CRconLog;
