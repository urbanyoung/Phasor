#include <windows.h>
#include <string>
#include <stdio.h>
#include <list>
#include "../Phasor/Common.h"
#include "../Phasor/Phasor.h"
#include <assert.h>

typedef std::map<std::string, Common::ObjectWrap> ThreadExecParam;

/* This class is used for a two way function invocation between threads. The
 * spawned thread invokes functions in the callee thread (who checks this
 * object) and the callee invokes in the spawned thread. 
 **/
class ThreadExec
{
private:
	// Struct used for storing information about function calls
	struct ThreadEvent
	{
		DWORD id;
		void (*callback)(ThreadExec*, ThreadExecParam&);
		ThreadExecParam data;

		ThreadEvent(DWORD id, void (*callback)(ThreadExec*, ThreadExecParam&),
			ThreadExecParam* data=NULL)
		{
			this->id = id;
			this->callback = callback;
			if (data) this->data = *data;
		}
	};

	DWORD exec_thread, callee_thread; // thread ids
	HANDLE creation_event, exec_event, callee_event, hThread; // handles used
	CRITICAL_SECTION cs; // used for locking the events list
	std::list<ThreadEvent*> events, tmp, *active; // active points to list to modify
	bool alive; // specifies whether the instance is being cleaned up
	
	// Thread owned by this object
	static DWORD WINAPI ThreadProc(LPVOID lParam)
	{
		ThreadExec* _this = (ThreadExec*)lParam;
		SetEvent(_this->creation_event);

		do {
			WaitForSingleObject(_this->exec_event, INFINITE);
		} while (_this->ProcessEvents(_this->exec_thread));

		return 0;
	}

	// Processes any events in the specified thread
	bool ProcessEvents(DWORD threadid)
	{
		EnterCriticalSection(&cs);
		active = &tmp; // so events isn't modified while we loop
		std::list<ThreadEvent*>::iterator itr = events.begin();
		while (itr != events.end())
		{
			if ((*itr)->id == threadid) {
				(*itr)->callback(this, (*itr)->data);
				delete *itr;
				itr = events.erase(itr);
			} else
				itr++;
		}
		
		// process any modifications made while processing the callbacks
		itr = tmp.begin();
		
		while (itr != tmp.end()) {
			printf("Pushing from tmp\n");
			events.push_back(*itr);
			itr++;
		}
		tmp.clear();
		active = &events;

		LeaveCriticalSection(&cs);
		return alive; // if false we'll cleanup
	}	

	// Frees the memory and clears the specified list
	void CleanupList(std::list<ThreadEvent*>& l)
	{
		std::list<ThreadEvent*>::iterator itr = l.begin();
		while (itr != l.end()) {
			delete *itr;
			itr = l.erase(itr);
		}
	}

public:
	
	ThreadExec(DWORD callee_thread=0) : alive(true)
	{
		if (!callee_thread) callee_thread = GetCurrentThreadId();
		this->callee_thread = callee_thread;

		active = &events;

		InitializeCriticalSection(&cs);		

		// used to signal when there is an event to process
		exec_event = CreateEvent(0, TRUE, FALSE, NULL);
		callee_event = CreateEvent(0, TRUE, FALSE, NULL);

		// Create the thread and wait until it's ready
		creation_event = CreateEvent(0, TRUE, FALSE, NULL);
		hThread = CreateThread(0, 0, ThreadProc, (LPVOID)this, 0, &exec_thread);
		WaitForSingleObject(creation_event, INFINITE);
		CloseHandle(creation_event);
	}

	~ThreadExec()
	{
		// notify the thread we're closing
		alive = false;
		SetEvent(exec_event);

		WaitForSingleObject(hThread, 1000); // thread notifies this when it closes

		CloseHandle(exec_event);
		CloseHandle(callee_event);

		CleanupList(events);
		CleanupList(tmp);

		DeleteCriticalSection(&cs);

		printf("ThreadExec cleaned up\n");
	}

	/* Invokes the callback function in associated thread. Any event to be
	 * invoked is guaranteed to be executed, even if the thread is to be closed
	 * after a request is made. */
	void Invoke(void (*callback)(ThreadExec*, ThreadExecParam&), 
		ThreadExecParam* data=NULL)
	{
		printf("Invoke\n");
		EnterCriticalSection(&cs);

		DWORD cur_thread = GetCurrentThreadId();
		if (cur_thread == callee_thread) { // send to exec thread
			active->push_back(new ThreadEvent(exec_thread, callback, data));
			SetEvent(exec_event);
		} else if (cur_thread == exec_thread) { // send to callee thread
			active->push_back(new ThreadEvent(callee_thread, callback, data));
			SetEvent(callee_event);
		}

		LeaveCriticalSection(&cs);
	}

	/* The controlling code should call this periodically to check for events.
	 * Multiple events may be executed per call.*/
	bool Check()
	{
		if (GetCurrentThreadId() != callee_thread) return false;
		if (WaitForSingleObject(callee_event, 0) == WAIT_OBJECT_0) // event
			return ProcessEvents(callee_thread);
		return true; 
	}

	/* Cleans up the instance and returns when done */
	static void Close(ThreadExec** t)
	{
		delete *t;
		*t = 0;
	}

	DWORD GetExecThread() const { return exec_thread;}
	DWORD GetCalleeThread() const { return callee_thread; }
};

void test(ThreadExec* t, ThreadExecParam& m)
{
	try {

		if (GetCurrentThreadId() == t->GetExecThread()) {
			printf("%i\n", m["test"].GetInt());
			t->Invoke(test, NULL);
		}
	} catch (Common::ObjectError & e)
	{
		printf("%s\n", e.what());
	}
}

int main()
{
	ThreadExec* t = new ThreadExec();
	
	int i = 0;
	while (t && t->Check()) {
		ThreadExecParam m;
		m["test"] = 135;
		i++;
		t->Invoke(test, NULL);
		if (i == 20000) {
			printf("Closing\n");
			ThreadExec::Close(&t);

		}
	}

	printf("Closing main thread\n");
	return 0;
}

#define DEFAULT_CACHE_SIZE		10

class Logging
{
private:
	// stores open logs and their associated ids
	static std::map<int, Logging*> logs;
	ThreadExec* exec_thread; // if non-null saving occurs in that thread
	std::list<std::string>* data;
	std::string log_name;
	CRITICAL_SECTION cs;
	size_t cache_size;
	// what is checked for caching decisions. Usually cache_size but can
	// be expanded if data wasn't saved.
	size_t use_cache_size; 

	/* log_name should be the absolute directory.
	 * NOTE: If a ThreadExec is specified then InvalidateThread() should
	 * be called before cleaning up this object. You must also make sure 
	 * that the ThreadExec queue is empty before cleaning up this object.*/
	Logging(const std::string& log_name, ThreadExec* t=0)
	{
		InitializeCriticalSection(&cs);
		this->exec_thread = t;
		data = new std::list<std::string>();

		this->log_name = log_name;/*Common::m_sprintf_s("%s\\%02i-%02i-%i_%02i-%02i-%02i_%s.log",
			Phasor::GetLogDirectory().c_str(),
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
			log_name.c_str());*/

		cache_size = DEFAULT_CACHE_SIZE;
		use_cache_size = cache_size;		 
	}

	/* once this is called it is assumed that exec_thread is no longer
	 * running and so this thread takes ownership of file objects */
	~Logging()
	{
		assert(exec_thread == 0);
		Logging::WriteData(log_name, data, NULL);
		// delete data; WriteData frees the memory
		DeleteCriticalSection(&cs);
	}

	void InvalidateThread() 
	{
		exec_thread = 0;
	}

	/* Called by WriteData if the file couldn't be opened. The list referred
	 * to by first is inserted at the start of any current data. */
	void MergeLists(std::list<std::string>& first)
	{
		EnterCriticalSection(&cs);
		use_cache_size += first.size();
		data->splice(data->begin(), first);		
		LeaveCriticalSection(&cs);
	}

	/* Called by WriteData when the requested data is successfully saved.*/
	void ResetCache()
	{
		EnterCriticalSection(&cs);
		use_cache_size = cache_size;
		LeaveCriticalSection(&cs);
	}
	
	/* writes the data to file, usually called from exec_thread if enabled */
	static void WriteData(const std::string& file, std::list<std::string>* data,
		Logging* owner, size_t retry_count=5) 
	{
		FILE* pFile = NULL;
		// Attempt to open th file until the number of failures has been reached
		size_t i = 0;
		while (i++ < retry_count && 
			(pFile = _fsopen(file.c_str(), "a+", _SH_DENYRW)) == NULL) 
		{
			Sleep(5);
		}

		// on failure add this data back to the list to try again later
		if (!pFile) {
			if (owner) owner->MergeLists(*data);
			delete data; // cleanup memory
			return;
		}

		std::list<std::string>::iterator itr = data->begin();
		while (itr != data->end()) {
			fprintf(pFile, "%s\n", itr->c_str());
			itr++;
		}

		fclose(pFile);
		delete data;

		if (owner) owner->ResetCache();
	}

	void LogData(const std::string& type, const std::string& name,
		const char* _Details, ...)
	{
		va_list ArgList;
		va_start(ArgList, _Details);
		std::string str = Common::FormatVarArgs(_Details, ArgList);
		va_end(ArgList);
		this->LogData(type, name, L"%s",
			Common::WidenString(str).c_str());
	}

	void LogData(const std::string& type, const std::string& name,
		const wchar_t* _Details, ...)
	{
		va_list ArgList;
		va_start(ArgList, _Details);
		std::wstring str = Common::WFormatVarArgs(_Details, ArgList);
		va_end(ArgList);

		// timestamp
		SYSTEMTIME st = {0};		
		GetLocalTime(&st);
		std::wstring timestamp = m_swprintf_s("")
	}

public:
	/* file should be the absolute path to the file (including file type) */
	/*
		put into namespace
	static void Create(int id, const std::string& file, ThreadExec* t=0);
	static void InvalidateThread(int id); // should be called before cleaning up
	static void Close(int id);
	static void SetMaxSize(int id, size_t size);
	static size_t GetMaxSize(int id);
	static void SetCacheSize(int id, int n);

	static void LogData(int id, const std::string& type, const std::string& name,
		const char* _Details, ...);
	static void LogData(int id, const std::string& type, const std::string& name,
		const wchar_t* _Details, ...);		*/
};

/*	Class for handling I/O safely between multiple processes. */
class FileWriter {
public:

	/* file should be the absolute path (including file type) */
	FileWriter(const std::string& file)  {

	}

};