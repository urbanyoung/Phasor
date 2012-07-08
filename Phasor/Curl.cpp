#include "Curl.h"
#include "../Phasor/Common.h"
#include <assert.h>

namespace Server
{
	namespace Curl
	{
		//-----------------------------------------------------------------------------------------
		// Class: CurlMulti
		//
		CurlMulti::CurlMulti()
		{
			multi_curl = curl_multi_init();
			running = 0;
			started = false;
		}

		CurlMulti::~CurlMulti()
		{
			std::list<CurlSimple*>::iterator itr = simples.begin();
			while (itr != simples.end()) {
				delete *itr;
				itr = simples.erase(itr);
			}
			curl_multi_cleanup(multi_curl);
		}

		bool CurlMulti::AddRequest(CurlSimple* simple_curl)
		{	
			CURLMcode c = curl_multi_add_handle(multi_curl, simple_curl->GetCurl());
			if (c != CURLM_OK) { // error
				SetError("Failed to add CurlSimple connection, error code " + c);
				return false;
			}
			if (!simple_curl->OnAdd()) {
				SetError("CurlSimple object is not ready to be processed.");
				return false;
			}
			simples.push_back(simple_curl);
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
					CurlSimple* simp = FindConnection(msg->easy_handle);
					assert(simp != 0); // never happen
					simp->ConnectionDone(msg);
					Remove(simp);
				}
				printf("%i\n", msg->msg);
			}

			running = new_running;
			return running != 0;
		}

		CurlSimple* CurlMulti::FindConnection(CURL* con)
		{
			std::list<CurlSimple*>::iterator itr = simples.begin();
			while (itr != simples.end()) {
				if ((*itr)->GetCurl() == con) return *itr;
				else itr++;
			}
			return NULL;
		}

		void CurlMulti::Remove(CurlSimple* simple_curl)
		{
			std::list<CurlSimple*>::iterator itr = simples.begin();
			while (itr != simples.end()) {
				if (*itr == simple_curl) {
					curl_multi_remove_handle(multi_curl, simple_curl->GetCurl());
					delete *itr;
					itr = simples.erase(itr);
					break;
				}
				else itr++;
			}
		}

		void CurlMulti::SetError(std::string error) 
		{ 
			this->errorMsg = error; 
			this->hasError = true;
		}

		//-----------------------------------------------------------------------------------------
		// Class: CurlSimple
		//
		CurlSimple::CurlSimple(std::string url)
		{
			init(url);
		}

		void CurlSimple::init(std::string url)
		{
			curl = curl_easy_init();
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Curl_OnDataWrite);
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

			url = url;
			buffer = new BYTE[DEFAULT_BUFFER_SIZE];
			completionRoutine = NULL;
			bufferSize = DEFAULT_BUFFER_SIZE;
			recvCount = 0;
			hasError = false;
		}

		CurlSimple::~CurlSimple()
		{
			printf("~CurlSimple()\n");
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
					printf("Error\n");
					SetError("OnDataWrite: Cannot allocate enough memory for received data.");
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
				SetError("Connection failed with error " + msg->data.result);
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

		void CurlSimple::SetError(std::string error) 
		{ 
			printf("Error: %s\n", error.c_str());
			this->errorMsg = error; 
			this->hasError = true;
		}

		//-----------------------------------------------------------------------------------------
		// Class: CurlSimpleHttp
		//
		CurlSimpleHttp::CurlSimpleHttp(std::string url) : CurlSimple(url)
		{
			pair_added = false;
			urlBuilder = new common::StreamBuilder();
			urlBuilder->AppendString(url.c_str());
			if (url[url.size()-1] != '?')
				urlBuilder->AppendString("?");
		}

		CurlSimpleHttp::~CurlSimpleHttp()
		{
			delete urlBuilder;
		}

		bool CurlSimpleHttp::OnAdd()
		{
			BYTE* stream = urlBuilder->getStream();
			stream[urlBuilder->getStreamSize()] = 0;
			url = (char*)stream;
			curl_easy_setopt(GetCurl(), CURLOPT_URL, url.c_str());
			return true;
		}

		void CurlSimpleHttp::AddPostData(std::string key, std::string data, bool b)
		{

		}

		void CurlSimpleHttp::AddPostData(std::string key, std::wstring data)
		{
			std::string escaped = Escape(data);
			AddPostData(key, escaped, true);
		}

		void CurlSimpleHttp::AddGetData(std::string key, std::string data, bool b)
		{
			if (!b) // not escaped
				data = Escape(data);
			key = Escape(key);
			if (pair_added)	
				urlBuilder->AppendString("&");
			urlBuilder->AppendString(key.c_str());
			urlBuilder->AppendString("=");
			urlBuilder->AppendString(data.c_str());
			pair_added = true;
		}

		void CurlSimpleHttp::AddGetData(std::string key, std::wstring data)
		{
			std::string escaped = Escape(data);
			AddGetData(key, escaped, true);
		}

		std::string CurlSimpleHttp::Escape(std::wstring input)
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

		std::string CurlSimpleHttp::Escape(std::string input)
		{
			std::string escaped;
			char* ptr = curl_easy_escape(GetCurl(), input.c_str(), input.size());
			escaped = ptr;
			curl_free(ptr);
			return escaped;
		}

		std::string CurlSimpleHttp::wstring_to_utf8_hex(const std::wstring &input)
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
		CurlSimpleDownload::CurlSimpleDownload(std::string url, 
			std::string path_to_file) : CurlSimple(url) 
		{
			file = path_to_file;
			pFile = fopen(file.c_str(), "wb");
			if (!pFile) {
				std::string err = "CurlSimpleDownload cannot create local file: " + this->file;
				throw std::exception(err.c_str());
			}
		}

		CurlSimpleDownload::~CurlSimpleDownload()
		{
			if (pFile)
				fclose(pFile);
		}

		size_t CurlSimpleDownload::OnDataWrite(BYTE* data, size_t size, size_t nmemb)
		{
			printf("Writing %i bytes to file. %08x\n", size*nmemb, this);
			return fwrite(data, size, nmemb, pFile);
		}

		void CurlSimpleDownload::ConnectionDone(CURLMsg* msg)
		{
			fclose(pFile);
			pFile = NULL;
			CurlSimple::ConnectionDone(msg);
		}
	}
}