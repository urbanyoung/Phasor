#pragma once
#include <list>
#include "../libcurl/curl/curl.h"
#include <sstream>
#include "Phasor.h"

namespace Server
{
	namespace Curl
	{
		class CurlMulti;
		class CurlSimple;
		class CurlHttp;
		class CurlDownload;

		typedef std::shared_ptr<CurlMulti> CurlMultiPtr;
		typedef std::shared_ptr<CurlSimple> CurlPtr;
		typedef std::shared_ptr<CurlHttp> CurlHttpPtr;
		typedef std::shared_ptr<CurlDownload> CurlDownloadPtr;

		//-----------------------------------------------------------------------------------------
		// Class: CurlMulti
		// Purpose: Manages a non-blocking (ish) multi curl interface. 
		class CurlMulti : public Phasor::Error
		{
		private:
			CURLM* multi_curl; // the multi stack used
			std::list<CurlPtr> simples; // simple connections tracked
			int running; // number of connections active
			bool started;

			/* Shouldn't be called directly */
			CurlMulti();

			/*	Adds a CurlSimple connection to tracking list. */
			bool AddRequest(CurlPtr simple_curl);

		protected:
			/* Returns the CurlSimple associated with the CURL connection */
			CurlPtr FindConnection(CURL* con);

		public:
			
			static CurlMultiPtr Create();
			~CurlMulti();			

			/*	Processes any waiting data
				Return: true (active connections), false (no active connections)*/
			bool Process();

			/*	Removes a CurlSimple connection */
			void Remove(CurlPtr simple_curl);

			friend class CurlSimple;
		};

		//-----------------------------------------------------------------------------------------
		// Class: CurlSimple
		// Purpose: Manage simple curl connections for use in CurlMulti.
		class CurlSimple : public std::enable_shared_from_this<CurlSimple>, 
			public Phasor::Error
		{
		private:
			CURL* curl; // the easy curl interface used
			static const unsigned long DEFAULT_BUFFER_SIZE = 4096;

			/*	Called by the curl object when there is data to be processed. This
				function is simply used to call OnDataWrite in a C++ context. */
			static size_t Curl_OnDataWrite(BYTE* data, size_t size, size_t nmemb, void* userdata);

			/* Called by CurlMulti when this instance is added to the multistack 
			 * Return values specifies whether or not it should be added. */
			virtual bool OnAdd() { return true; }

			/* Returns a shared pointer to this object */
			CurlPtr get_shared();

		protected:
			std::string url; // url the connection is to be made on
			BYTE* buffer; // buffer containing all data received
			size_t recvCount; // number of bytes received
			size_t bufferSize; // size of the allocated buffer
			void (*completionRoutine)(BYTE*, size_t, void* userdata); // ptr to function called on completion
			void* userdata; // set by user for passing to completion routine

			/* Returns the pointer to this curl instance */
			CURL* GetCurl() { return curl; };

			/*  Processes data received from the curl interface.
				Unless overridden this function will store all received data in
				the internal buffer. As such this method should be overridden if
				the expected data is too large to store into memory.
				The internal buffer is expanded as required.
			*/
			virtual size_t OnDataWrite(BYTE* data, size_t size, size_t nmemb);

			/*	Called by CurlMulti when the current operation has completed, 
				successfully or not. */
			virtual void ConnectionDone(CURLMsg* msg);

			/* Shouldn't be called directly */
			CurlSimple(const std::string& url);

		public:

			static CurlPtr Create(const std::string& url);
			virtual ~CurlSimple();

			/* Initializes the class */
			void init(const std::string& url);

			/* Associates this with a specified multi connection */
			void Associate(CurlMultiPtr multi);

			/*	Sets the user defined function to call upon request completion.
				The data passed to the function is this object's internal buffer
				and should not be modified. Once the function completes	the 
				buffer's state is not guaranteed to be consistent and as such the 
				data should be copied into a new buffer if required. */
			virtual void RegisterCompletion(void (*function)(BYTE*, size_t, void*), void* userdata=0);	
		
			/*	Resizes the internal buffer which is used for storing received
				data. 
				If new_size is smaller than the current buffer size, only
				the first new_size bytes are copied into the new buffer.*/
			bool ResizeBuffer(size_t new_size);

			friend class CurlMulti;
		};

		//-----------------------------------------------------------------------------------------
		// Class: CurlSimpleHttp
		// Purpose: Connects to and downloads a web page and (optionally) sends get or post data
		class CurlHttp : public CurlSimple
		{
		private:
			bool pair_added; // should & be prepended
			std::stringstream ssurl; // used for building get urls
			curl_httppost* form; // used for posts
			curl_httppost* last;

			/* Helper function for escaping strings */
			std::string wstring_to_utf8_hex(const std::wstring &input);

			/* Escapes the input string for url encoding */
			std::string Escape(const std::wstring& input);
			std::string Escape(const std::string& input);

			/* Called by CurlMulti when this instance is added to the multistack 
			 * Return values specifies whether or not it should be added. */
			virtual bool OnAdd();

			CurlHttp(const std::string& url);
		public:

			static CurlHttpPtr Create(const std::string& url);
			virtual ~CurlHttp();

			/* Adds data to the http request, any unicode strings are escaped. */
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
		private:
			std::string file;
			FILE* pFile;

			CurlDownload(const std::string& url, const std::string& path_to_file);

		protected:
			/*	Called by CurlMulti when the file has downloaded */
			virtual void ConnectionDone(CURLMsg* msg);

			/*	Writes any received data to file with fwrite. */
			virtual size_t OnDataWrite(BYTE* data, size_t size, size_t nmemb);

		public:
			static CurlDownloadPtr Create(const std::string& url, 
				const std::string& path_to_file);
			
			~CurlDownload();
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
}