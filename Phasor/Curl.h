#pragma once
#include <list>
#include "../libcurl/curl/curl.h"
#include "../Phasor/Common.h"

namespace Server
{
	namespace Curl
	{
		class CurlSimple;

		//-----------------------------------------------------------------------------------------
		// Class: CurlMulti
		// Purpose: Manages a non-blocking (ish) multi curl interface. 
		// All simple connections managed by this interface should not be modified
		// outside it. Upon request completion, the simple connection is cleaned
		// up.
		class CurlMulti
		{
		private:
			CURLM* multi_curl; // the multi stack used
			std::list<CurlSimple*> simples; // simple connections tracked
			int running; // number of connections active
			bool started, hasError; 
			std::string errorMsg;

			/* Sets the error */
			void SetError(std::string error);

		protected:
			/* Returns the CurlSimple associated with the CURL connection */
			CurlSimple* FindConnection(CURL* con);

		public:
			CurlMulti();
			~CurlMulti();

			/*	Adds a CurlSimple connection to tracking list. */
			bool AddRequest(CurlSimple* simple_curl);

			/*	Processes any waiting data
				Return: true (active connections), false (no active connections)*/
			bool Process();

			/*	Removes a CurlSimple connection */
			void Remove(CurlSimple* simple_curl);

			/*	Checks if there has been an unrecoverable error	*/
			bool IsError() { return hasError;};

			/* Returns the last unrecoverable error */
			std::string GetError() { return errorMsg;};
		};

		//-----------------------------------------------------------------------------------------
		// Class: CurlSimple
		// Purpose: Manage simple curl connections for use in CurlMulti.
		class CurlSimple
		{
		private:
			CURL* curl; // the easy curl interface used
			static const unsigned long DEFAULT_BUFFER_SIZE = 4096;
			std::string errorMsg; // stores the latest error
			bool hasError; // indicates whether or not there's been an error

			/*	Called by the curl object when there is data to be processed. This
				function is simply used to call OnDataWrite in a C++ context. */
			static size_t Curl_OnDataWrite(BYTE* data, size_t size, size_t nmemb, void* userdata);

			/* Called by CurlMulti when this instance is added to the multistack 
			 * Return values specifies whether or not it should be added. */
			virtual bool OnAdd() { return true; }

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

			/* Sets a message describing the last (fatal) error */
			void SetError(std::string error);

		public:

			CurlSimple(std::string url);
			~CurlSimple();

			/* Initializes the class */
			void init(std::string url);

			/*	Sets the user defined function to call upon request completion.

				The data passed to the function is this object's internal buffer
				and should not be modified. Once the function completes	the 
				buffer's state is not guaranteed to be consistent and as such the 
				data should be copied into a new buffer if required. */
			virtual void RegisterCompletion(void (*function)(BYTE*, size_t, void*), void* userdata=0);	
		
			/*	Resizes the internal buffer which is used for storing received
				data. 

				If new_size is smaller than the current buffer size, only
				the first new_size bytes are copied into the new buffer.
			*/
			bool ResizeBuffer(size_t new_size);

			/*	Checks if there has been an unrecoverable error	*/
			bool IsError() { return hasError;};

			/* Returns the last unrecoverable error */
			std::string GetError() { return errorMsg;};

			friend class CurlMulti;
		};

		//-----------------------------------------------------------------------------------------
		// Class: CurlSimpleHttp
		// Purpose: Connects to and downloads a web page and (optionally) sends get or post data
		class common::StreamBuilder;
		class CurlSimpleHttp : public CurlSimple
		{
		private:
			bool pair_added; // should & be prepended
			common::StreamBuilder* urlBuilder;

			/* Helper function for escaping strings */
			std::string wstring_to_utf8_hex(const std::wstring &input);

			/* Escapes the input string for url encoding */
			std::string Escape(std::wstring input);
			std::string Escape(std::string input);

			/* Called by CurlMulti when this instance is added to the multistack 
			 * Return values specifies whether or not it should be added. */
			virtual bool OnAdd();

		public:
			CurlSimpleHttp(std::string url);
			~CurlSimpleHttp();

			/* Adds data to the http request, any unicode strings are escaped. */
			void AddPostData(std::string key, std::wstring data);
			void AddPostData(std::string key, std::string data, bool b=false);
			void AddGetData(std::string key, std::wstring data);
			void AddGetData(std::string key, std::string data, bool b=false);
		};

		//-----------------------------------------------------------------------------------------
		// Class: CurlSimpleDownload
		// Purpose: Downloads the file at the specified url
		class CurlSimpleDownload : public CurlSimple
		{
		private:
			std::string file;
			FILE* pFile;

		protected:
			/*	Called by CurlMulti when the file has downloaded */
			virtual void ConnectionDone(CURLMsg* msg);

			/*	Writes any received data to file with fwrite. */
			virtual size_t OnDataWrite(BYTE* data, size_t size, size_t nmemb);

		public:
			CurlSimpleDownload(std::string url, std::string path_to_file);
			~CurlSimpleDownload();
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