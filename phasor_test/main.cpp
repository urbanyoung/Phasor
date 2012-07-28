#include <windows.h>
#include <string>
#include <stdio.h>
#include <list>

class ThreadExec;

typedef std::shared_ptr<ThreadExec> ThreadExecPtr;

/* This class is used for a two way function invocation between threads. The
 * spawned thread invokes functions in the callee thread (who checks this
 * object) and the callee invokes in the spawned thread. 
 **/
class ThreadExec
{
private:
	DWORD exec_thread, callee_thread;
	HANDLE creation_event;
	CRITICAL_SECTION cs;

	struct thread_event
	{
		 void (*callback)(ThreadExec*, void*);
		 // generic value wrapper class, maybe move SQLiteValue into
		 // generic value class and have SQLiteValue inherit from it
	};

	static DWORD WINAPI ThreadProc(LPVOID lParam)
	{
		ThreadExec* _this = (ThreadExec*)lParam;
		SetEvent(_this->creation_event);

		while (_this->Check()) Sleep(20);

		return 0;
	}

	void ProcessEvents(DWORD threadid)
	{

	}

public:
	ThreadExec(DWORD callee_thread = 0)
	{
		if (!callee_thread) callee_thread = GetCurrentThreadId();
		this->callee_thread = callee_thread;

		InitializeCriticalSection(&cs);		

		// Create the thread and wait until it's ready
		creation_event = CreateEvent(0, TRUE, FALSE, NULL);
		CreateThread(0, 0, ThreadProc, (LPVOID)this, 0, &exec_thread);
		WaitForSingleObject(creation_event, INFINITE);
		CloseHandle(creation_event);
	}

	~ThreadExec()
	{
		// kill thread
	}

	/* Invokes the callback function in associated thread. */
	void Invoke(void* userdata, void (*callback)(ThreadExec*, void*))
	{
		EnterCriticalSection(&cs);

		DWORD cur_thread = GetCurrentThreadId();
		if (cur_thread == exec_thread) {

		} else if (cur_thread == callee_thread) {

		}

		LeaveCriticalSection(&cs);
	}

	/* Checks the object for events to process, should be called by the 
	 * "callee" thread periodically. */
	bool Check()
	{

	}
};

/*class Logging : public ThreadExec
{
private:
	// stores open logs and their associated ids
	static std::map<int, Logging*> logs;
	DWORD exec_thread; // if non-null saving occurs in that thread

	Logging(DWORD exec_thread)
	{

	}

	~Logging();

	void LogData(const std::string& type, const std::string& name,
		const char* _Details, ...);
	void LogData(const std::string& type, const std::string& name,
		const wchar_t* _Details, ...);

public:
	static void Create(int id, const std::string& file, const DWORD exec_thread = 0);
	static void Close(int id);
	static void SetMaxSize(int id, size_t size);
	static size_t GetMaxSize(int id);

	static void LogData(int id, const std::string& type, const std::string& name,
		const char* _Details, ...);
	static void LogData(int id, const std::string& type, const std::string& name,
		const wchar_t* _Details, ...);		
};
*/
int main()
{
	
	return 0;
}