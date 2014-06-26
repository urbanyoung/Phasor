/*! \file scripttimers.h
 *	\brief Timer related functions for scripts.
 *
 *	\addtogroup PhasorAPI
 *	@{
 */
#pragma once
#include "../phasor-lua.hpp"

/*! \brief Registers a timer which is called when it expires.
 *
 *	\param delay The delay (in milliseconds) to wait before calling \c callback.
 *	\param callback A string representing the function to call once the delay expires.
 *	\param [userdata] A variable you want passed to the callback.
 *	\return The created timer's id, which can be used in \c removetimer
 *
 *	\remark
 *		- \c registertimer works slightly differently that it did before. You can
 *		only specify one \c userdata value, however <b>tables are supported.</b>
 *		- The callback function always receives three parameters: \c id, \c call_count
 *		and \c userdata. \c id is the timer's id (same value registertimer returns),
 *		\c call_count is the number of times this timer has been called (starting at 1)
 *		and \c userdata is the value passed to \c registertimer.
 *		- The callback should return true if you want to renew the timer.
 *
 *	Example usage:
 *	\code
 *		registertimer(1000, "MyTimerCallback", {"hello", 1, 2,3})
 *
 *		function MyTimerCallback(id, call_count, userdata)
 *			hprintf(userdata[1])
 *			return true -- renew the timer
 *		end
 *	\endcode
 */
int l_registertimer(lua_State* L);

/*! \brief Removes a currently registered timer.
 *
 *	\param id The id of the timer to remove.
 *
 *	\remark
 *		- \c id can be invalid, in which case the call is ignored (no errors are raised)
 *		- If you want to remove a timer from its callback, return \c false. Don't
 *		use removetimer.
 *
 *	Example usage:
 *	\code
 *		removetimer(timer_id)
 *	\endcode
 */
int l_removetimer(lua_State* L);

//! }@