#include "Curl.h"
#include "../Phasor/Common/Common.h"

namespace Curl
{
	//-----------------------------------------------------------------------------------------
	// Class: CurlMulti
	//
	CurlMulti::CurlMulti() : running(0), started(false)
	{
		multi_curl = curl_multi_init();
		if (!multi_curl) //throw std::exception();
			HandleError("couldn't init curl_multi interface");
	}

	CurlMulti::~CurlMulti()
	{
		curl_multi_cleanup(multi_curl);
	}

	bool CurlMulti::AddRequest(CurlPtr simple_curl)
	{	
		CURLMcode c = curl_multi_add_handle(multi_curl, simple_curl->GetCurl());
		if (c != CURLM_OK) { // error
			std::stringstream ss;
			ss << "Failed to add CurlSimple connection, error code : " << c;
			HandleError(ss.str());
			return false;
		}
		if (!simple_curl->OnAdd()) {
			curl_multi_remove_handle(multi_curl, simple_curl->GetCurl());
			HandleError("CurlSimple object is not ready to be processed.");
			return false;
		}
		simples.push_back(std::move(simple_curl));
		return true;
	}

	bool CurlMulti::Process()
	{
		int new_running = running;
		CURLMcode c = curl_multi_perform(multi_curl, &new_running);

		CURLMsg* msg = 0;
		int msgs_left = 1;
		while (msgs_left && (msg = curl_multi_info_read(multi_curl, &msgs_left))) {
			if (msg->msg == CURLMSG_DONE) {			
				CurlSimple* simp = 0;
				FindConnection(msg->easy_handle, &simp);
				simp->ConnectionDone(msg);
				Remove(&simp);
			}
		}

		running = new_running;
		return running != 0;
	}

	bool CurlMulti::FindConnection(CURL* con, CurlSimple** out)
	{
		std::list<CurlPtr>::iterator itr = simples.begin();
		while (itr != simples.end()) {
			if ((*itr)->GetCurl() == con) {
				*out = (*itr).get();
				return true;
			}
			else itr++;
		}
		return false;
	}

	void CurlMulti::Remove(CurlSimple** simple_curl)
	{
		if (!simple_curl) return;
		std::list<CurlPtr>::iterator itr = simples.begin();
		while (itr != simples.end()) {
			if ((*itr).get() == *simple_curl) {
				curl_multi_remove_handle(multi_curl, (*simple_curl)->GetCurl());
				itr = simples.erase(itr);
				break;
			}
			else itr++;
		}
		simple_curl = NULL;
	}

	void CurlMulti::HandleError(const std::string& err)
	{
		printf("multi: %s\n", err.c_str());
	}
	//-----------------------------------------------------------------------------------------
	// Class: CurlSimple
	//
	CurlSimple::CurlSimple(const std::string& url)
		: url(url), completionRoutine(NULL),
		bufferSize(DEFAULT_BUFFER_SIZE), recvCount(0)
	{
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Curl_OnDataWrite);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 5);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 10);

		buffer = new BYTE[DEFAULT_BUFFER_SIZE];
	}

	CurlSimple::~CurlSimple()
	{
		curl_easy_cleanup(curl);
		delete[] buffer;
	}

	size_t CurlSimple::Curl_OnDataWrite(BYTE* data, size_t size, size_t nmemb, void* userdata)
	{
		if (!size) return 0; // can be called with no data
		CurlSimple* simple = (CurlSimple*)userdata;
		return simple->OnDataWrite(data, size, nmemb);
	}

	size_t CurlSimple::OnDataWrite(BYTE* data, size_t size, size_t nmemb)
	{
		size_t nbytes = size*nmemb;

		if (recvCount + nbytes > bufferSize) {
			if (!ResizeBuffer(bufferSize + nbytes))	{
				HandleError("cannot allocate enough memory for received data.");
				return 0; // curl will abort
			}
		}

		memcpy(buffer + recvCount, data, nbytes);
		recvCount += nbytes;
		return nbytes;
	}

	void CurlSimple::ConnectionDone(CURLMsg* msg)
	{
		if (msg->data.result != CURLM_OK) {
			std::stringstream ss;
			ss << "Connection failed with error: " << msg->data.result;
			HandleError(ss.str());
			recvCount = 0;
			return;
		}

		if (completionRoutine) completionRoutine(buffer, recvCount, userdata);	
	}

	void CurlSimple::RegisterCompletion(void (*function)(BYTE*, size_t, void*), void* userdata)
	{
		completionRoutine = function;
		this->userdata = userdata;
	}

	bool CurlSimple::ResizeBuffer(size_t new_size)
	{
		bool success = true;
		try 
		{
			BYTE* new_buffer = new BYTE[new_size];
			size_t ncopy = (new_size < bufferSize) ? new_size : bufferSize;
			for (size_t x = 0; x < ncopy; x++)	new_buffer[x] = buffer[x];
			delete[] buffer;
			buffer = new_buffer;
			bufferSize = new_size;
		}
		catch (std::bad_alloc &)
		{
			success = false;			
		}
		return success;
	}

	void CurlSimple::HandleError(const std::string& err)
	{
		printf("simple: %s\n", err.c_str());
	}

	//-----------------------------------------------------------------------------------------
	// Class: CurlSimpleHttp
	//
	CurlHttp::CurlHttp(const std::string& url) 
		: CurlSimple(url), pair_added(false), form(NULL),
		last(NULL)
	{
		ssurl << url;
		if (url[url.size()-1] != '?')
			ssurl << "?";
	}

	CurlHttp::~CurlHttp()
	{
		if (form) curl_formfree(form);
	}

	bool CurlHttp::OnAdd()
	{
		url = ssurl.str();
		curl_easy_setopt(GetCurl(), CURLOPT_URL, url.c_str());
		curl_easy_setopt(GetCurl(), CURLOPT_HTTPPOST, form);
		return true;
	}

	void CurlHttp::AddPostData(const std::string& key, const std::string& data, bool b)
	{
		std::string post_data = data;
		if (!b) // not escaped
			post_data = Escape(data);

		curl_formadd(&form, &last, CURLFORM_COPYNAME, key.c_str(),
			CURLFORM_COPYCONTENTS, post_data.c_str(), CURLFORM_END);
	}

	void CurlHttp::AddPostData(const std::string& key, const std::wstring& data)
	{
		std::string escaped = Escape(data);
		AddPostData(key, escaped, true);
	}

	void CurlHttp::AddPostFile(const std::string& key, const std::string& path_to_file)
	{
		curl_formadd(&form, &last, CURLFORM_COPYNAME, key.c_str(), CURLFORM_FILE, path_to_file.c_str(), CURLFORM_END);			
	}

	void CurlHttp::AddGetData(const std::string& key, const std::string& data, bool b)
	{
		std::string get_data = data;
		if (!b) // not escaped
			get_data = Escape(data);
		std::string get_key = Escape(key);
		if (pair_added)	
			ssurl << "&";
		ssurl << get_key << "=" << get_data;
		pair_added = true;
	}

	void CurlHttp::AddGetData(const std::string& key, const std::wstring& data)
	{
		std::string escaped = Escape(data);
		AddGetData(key, escaped, true);
	}

	std::string CurlHttp::Escape(const std::wstring& input)
	{
		std::string escaped;
		for (size_t x = 0; x < input.size(); x++) {
			wchar_t c = input[x];
			if (!(c >= L'A' && c <= L'Z') && !(c >= L'a' && c <= L'z')
				&& !(c >= L'0' && c <= L'9') && c != L'.' && c != L'_'
				&& c != L'-' && c != '~')
				escaped += wstring_to_utf8_hex(std::wstring(&c));			
			else
				escaped += (char)c;
		}

		return escaped;
	}

	std::string CurlHttp::Escape(const std::string& input)
	{
		std::string escaped;
		char* ptr = curl_easy_escape(GetCurl(), input.c_str(), input.size());
		escaped = ptr;
		curl_free(ptr);
		return escaped;
	}

	std::string CurlHttp::wstring_to_utf8_hex(const std::wstring &input)
	{
		// http://stackoverflow.com/questions/3300025/how-do-i-html-url-encode-a-stdwstring-containing-unicode-characters
		std::string output;
		int cbNeeded = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), 
			-1, NULL, 0, NULL, NULL);
		if (cbNeeded > 0) {
			char *utf8 = new char[cbNeeded];
			if (WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1,
				utf8, cbNeeded, NULL, NULL) != 0) {
					for (char *p = utf8; *p; *p++) {
						char onehex[5];
						sprintf_s(onehex, sizeof(onehex), "%%%02.2X", (unsigned char)*p);
						output.append(onehex);
					}
			}
			delete[] utf8;
		}

		return output;
	}

	//-----------------------------------------------------------------------------------------
	// Class: CurlSimplDownload
	//
	CurlDownload::CurlDownload(const std::string& url, 
		FILE* pFile) 
		: CurlSimple(url), pFile(pFile)
	{
	}

	CurlDownload::~CurlDownload()
	{
		if (pFile) fclose(pFile);
	}

	FILE* CurlDownload::OpenOutputFile(const std::string& file)
	{
		return fopen(file.c_str(), "wb");
	}

	size_t CurlDownload::OnDataWrite(BYTE* data, size_t size, size_t nmemb)
	{
		return fwrite(data, size, nmemb, pFile);
	}

	void CurlDownload::ConnectionDone(CURLMsg* msg)
	{
		fclose(pFile);
		pFile = NULL;
		CurlSimple::ConnectionDone(msg);
	}
}