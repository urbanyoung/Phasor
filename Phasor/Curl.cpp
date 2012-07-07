#include "Curl.h"
#include "Common.h"
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
			}

			running = new_running;
			return running != 0;
		}

		CurlSimple* CurlMulti::FindConnection(CURL* con)
		{
			std::list<CurlSimple*>::iterator itr = simples.begin();
			while (itr != simples.end()) {
				if ((*itr)->GetCurl() == con) return *itr;
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
		CurlSimpleHttp::CurlSimpleHttp(std::string url, CSIMPLE_HTTP mode) : CurlSimple(url)
		{

		}

		CurlSimpleHttp::~CurlSimpleHttp()
		{

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
			printf("Writing %i bytes to file.\n", size*nmemb);
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