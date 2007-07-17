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

    /*!@class FutureValuePending
    ** @brief thrown when Future::getValue is called on a Future object which has not yet been computed.
    ** @remark
    **   Todo: Improve on the base exception class, to provide better message passing options
    */
    class FutureValuePending : public std::exception
    {
    public:
        FutureValuePending()
        {}

        FutureValuePending(const char *const& _What)
            : std::exception(_What)
        {}
    };
}