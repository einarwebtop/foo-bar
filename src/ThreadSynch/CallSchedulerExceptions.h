/************************************************************************
** Copyright 2007 Einar Otto Stangvik
** 
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** 
**    http://www.apache.org/licenses/LICENSE-2.0
** 
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#pragma once

namespace ThreadSynch
{
    /************************************************************************
    ** Exceptions
    */

    /*!@class CallSchedulingFailedException
    ** @brief thrown when some undefined factor stops the scheduled call from running.
    ** @remark
    **   Todo: Improve on the base exception class, to provide better message passing options
    */
    class CallSchedulingFailedException : public std::exception
    {
    public:
        CallSchedulingFailedException()
        {}

        CallSchedulingFailedException(const char *const& _What)
            : std::exception(_What)
        {}
    };

    /*!@class CallTimeoutException
    ** @brief thrown when a scheduled call times out.
    */
    class CallTimeoutException : public std::exception
    {
    public:
        CallTimeoutException()
        {}

        CallTimeoutException(const char *const& _What)
            : std::exception(_What)
        {}
    };

    /*!@class UnexpectedException
    ** @brief thrown when a scheduled call throws an exception which wasn't expected by the user.
    */
    class UnexpectedException
    {
    public:
        UnexpectedException()
        {}
    };
}