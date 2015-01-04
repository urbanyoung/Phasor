/*! \file http.h
*	\brief Functions related to HTTP support for scripts.
*
*	\addtogroup PhasorAPI
*	@{
*/
#pragma once
#include "../phasor-lua.hpp"
#include "../scripting.hpp"

/*! 
* \brief Perform an HTTP operation asynchronously and call the specified
* callback when done. 
*
* \param url url to send the request to
* \param callback function to call when the operation has completed, see remarks.
* \param [userdata] userdata passed to callback
* \param [mode] type of operation to perform, see remarks.
* \param [values] key / value mapping of values to send. Note: only types strings and numbers are accepted.
* \param [headers] key / value mappings for HTTP headers.
*
* \remark
* Phasor defines the following variables that can be used for the \c mode parameter: \c HTTP_GET, \c HTTP_POST, \c HTTP_PUT, \c HTTP_HEAD.
* If undefined, mode defaults to \c HTTP_GET.
*
* \remark
* The callback function's signature should match:
* \code
*    function f(result, headers, body, userdata)
* \endcode
* Where \c result is the HTTP result code (e.g. 404), \c headers is an array of key/value pairs
* and body is the string returned by the server.
*
* \remark
* HTTP events prolong the life of a script. If a script is to be unloaded and it has outstanding HTTP events,
* then OnScriptUnload is called, but the script is not unloaded. However, it will no longer receive any
* event notifications and it can only call the following API functions: hprintf, respond, log_msg, tokenizestring
* and tokenizecmdstring. Scripts that are waiting for HTTP requests do not show up in sv_script_list and 
* can have no further impact on the game (they are, for all intents and purposes, unloaded). 
*
* \remark
* HTTP requests should not return large amounts of data to Phasor. Every HTTP request has a 
* 1MB limit for any received data; if this is exceeded then the request will fail with error
* -1 and body will be the first 1MB of data received.
*	Example usage:
*	\code
write me!
*	\endcode
*/
int l_http(lua_State* L);

int l_httpraw(lua_State* L);

namespace scripting {
    namespace http_requests {

        void setupScript(PhasorScript& script);
        void checkRequests();
    }
}

//! }@