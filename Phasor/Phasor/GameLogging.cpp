#include "GameLogging.h"
#include "../Common/MyString.h"
#include "Directory.h"

// ------------------------------------------------------------------------
// CGameLog
// 
static const wchar_t* type_names[] = 
{
	L"   GAME     ",
	L"   PLAYER   ",
	L"   SERVER   "
};

static const wchar_t* event_names[] = 
{
	L"END          ",
	L"START        ",
	L"JOIN         ",
	L"LEAVE        ",
	L"CHANGE       ",
	L"DEATH        ",
	L"COMMAND      ",
	L"CLOSE        "
};

CGameLog::CGameLog(const std::wstring& dir, const std::wstring& file,
	PhasorThread& thread)
	: logstream(new CThreadedLogging(dir, file, g_OldLogsDirectory, thread, kSaveDelay))
{
}

void CGameLog::WriteLog(glog_type type, char* format, ...)
{
	va_list ArgList;
	va_start(ArgList, format);
	std::string str = FormatVarArgs(format, ArgList);
	va_end(ArgList);
	return WriteLog(type, L"%s", WidenString(str).c_str());
}

void CGameLog::WriteLog(glog_type type, wchar_t* format, ...)
{
	va_list ArgList;
	va_start(ArgList, format);
	std::wstring str = FormatVarArgsW(format, ArgList);
	va_end(ArgList);

	DWORD type_name_index = ((type & kPlayerJoin) >> 7) + 
		((type & kServerCommand) >> 7);

	const wchar_t* type_name = type_names[type_name_index];
	const wchar_t* event_name = event_names[type & 0x3F];

	*logstream << type_name << event_name << str << endl;
}
