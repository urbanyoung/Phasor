#pragma once

#include <sstream>
#include <vector>
#include <list>
#include <memory>
#include "../Common/MyException.h"
#include "../../libcurl/curl/curl.h"

namespace Curl
{
	class CurlMulti;
	class CurlSimple;
	class CurlHttp;
	class CurlDownload;

	typedef std::unique_ptr<CurlMulti> CurlMultiPtr;
	typedef std::unique_ptr<CurlSimple> CurlPtr;
	typedef std::unique_ptr<CurlHttp> CurlHttpPtr;
	typedef std::unique_ptr<CurlDownload> CurlDownloadPtr;

	//-----------------------------------------------------------------------------------------
	// Class: CurlMulti
	// Purpose: Manages a non-blocking (ish) multi curl interface. 
	class CurlMulti
	{
	private:
		CURLM* multi_curl; // the multi stack used
		std::list<CurlPtr> simples; // simple connections tracked
		int running; // number of connections active
		bool started;

	protected:
		// Returns the CurlSimple associated with the CURL connection
		bool FindConnection(CURL* con, CurlSimple** out);

	public:
			
		CurlMulti();
		~CurlMulti();			

		virtual void HandleError(const std::string& err);

		//	Processes any waiting data
		//	Return: true (active connections), false (no active connections)
		bool Process();

		//	Removes a CurlSimple connection 
		void Remove(CurlSimple** simple_curl);

		// Add a request to the multi interface
		bool AddRequest(CurlPtr simple_curl);

		friend class CurlSimple;
	};

	//-----------------------------------------------------------------------------------------
	// Class: CurlSimple
	// Purpose: Manage simple curl connections for use in CurlMulti.
	class CurlSimple
	{
	private:
		CURL* curl; // the easy curl interface used
		static const unsigned long DEFAULT_BUFFER_SIZE = 4096;
		std::vector<BYTE> buffer;

		// Called by the curl object when there is data to be processed. This
		// function is simply used to call OnDataWrite in a C++ context.
		static size_t Curl_OnDataWrite(BYTE* data, size_t size, size_t nmemb, void* userdata);

	protected:
		std::string url; // url the connection is to be made on
		
		// Returns the pointer to this curl instance
		CURL* GetCurl() { return curl; };

		// Processes data received from the curl interface.
		// Unless overridden this function will store all received data in
		// the internal buffer. As such this method should be overridden if
		// the expected data is too large to store into memory.
		// The internal buffer is expanded as required.
		virtual size_t OnDataWrite(BYTE* data, size_t size, size_t nmemb);

		// Called by CurlMulti when the current operation has completed, 
		// successfully or not. The connect is cleaned up after this call.
		void ConnectionDone(CURLMsg* msg);

		// Classes should implement these functions for custom completion,
		// or error handling. data may be null if the internal buffer wasn't
		// used (ie in CurlDownload)
		virtual void OnCompletion(bool success, const BYTE* data, size_t size) {}

		// Called by CurlMulti when this instance is added to the multistack 
		// Return values specifies whether or not it should be added. 
		virtual bool OnAdd() { return true; }

	public:

		CurlSimple(const std::string& url);
		virtual ~CurlSimple();

		virtual void HandleError(const std::string& err);

		friend class CurlMulti;
	};

	//-----------------------------------------------------------------------------------------
	// Class: CurlSimpleHttp
	// Purpose: Connects to and downloads a web page and (optionally) sends get or post data
	class CurlHttp : public CurlSimple
	{
	private:
		bool pair_added, do_post; // should & be prepended
		std::stringstream ssurl; // used for building get urls
		curl_httppost* form; // used for posts
		curl_httppost* last;

		// Helper function for escaping strings
		std::string wstring_to_utf8_hex(const std::wstring &input);

		// Escapes the input string for url encoding 
		std::string Escape(const std::wstring& input);
		std::string Escape(const std::string& input);

		// Called by CurlMulti when this instance is added to the multistack 
		// Return values specifies whether or not it should be added.
		virtual bool OnAdd();
		
	public:

		CurlHttp(const std::string& url);
		virtual ~CurlHttp();

		// Adds data to the http request, any unicode strings are escaped. 
		void AddPostData(const std::string& key, const std::wstring& data);
		void AddPostData(const std::string& key, const std::string& data, bool b=false);
		void AddPostFile(const std::string& key, const std::string& path_to_file);
		void AddGetData(const std::string& key, const std::wstring& data);
		void AddGetData(const std::string& key, const std::string& data, bool b=false);			
	};

	//-----------------------------------------------------------------------------------------
	// Class: CurlSimpleDownload
	// Purpose: Downloads the file at the specified url
	class CurlDownload : public CurlSimple
	{
	protected:
		std::string file;
		FILE* pFile;

		//	Writes any received data to file with fwrite.
		virtual size_t OnDataWrite(BYTE* data, size_t size, size_t nmemb);

		virtual void OnCompletion(bool success, const BYTE*, size_t size);

	public:
		CurlDownload(const std::string& url, FILE* pFile);
		virtual ~CurlDownload();
		static FILE* OpenOutputFile(const std::string& file);	
	};


	//-----------------------------------------------------------------------------------------
	// Class: CurlSimpleFtp
	// Purpose: Connects to an ftp server and uploads/downloads file(s)
	/*class CurlSimpleFtp : CurlSimple
	{
	private:
		std::string outFile;

	public:
		CurlSimpleFtp(const char* outFile);
		~CurlSimpleFtp();
	};*/
}