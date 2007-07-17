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

#include "Future_Impl.h"

namespace ThreadSynch
{
    template<typename T>
    class Future
    {
    public:
        /*! 
        ** @brief Constructs a new instance of the Future template, capable of holding a value of inner type T.
        ** @warning The inner type cannot be of pointer or reference type. This restriction is imposed as an attempt to
        **          prevent the return value to share resources with other threads. It is, of course, not a fool proof
        **          guard, so be vary.
        ** @param[in] abortCallback a callback to a function which aborts the computation of the future variable.
        ** @param[in] waitCallback a callback which waits a number of milliseconds for the computation to take place.
        ** @param[in] getReturnValueCallback a callback which returns the computed future variable.
        ** @throw std::bad_alloc The inner Future_Impl could not be allocated.
        */
        Future(typename Future_Impl<T>::ABORTCALLBACKTYPE abortCallback,
               typename Future_Impl<T>::WAITCALLBACKTYPE waitCallback,
               typename Future_Impl<T>::GETRETURNVALUECALLBACKTYPE getReturnValueCallback)
               : m_pFutureImpl(new Future_Impl<T>(abortCallback, waitCallback, getReturnValueCallback))
        {}

        Future(const Future& other)
            : m_pFutureImpl(other.m_pFutureImpl)
        {}

        /*! 
        ** @brief Waits for the future computation to take place.
        ** @param[in] dwTimeout number of milliseconds to wait before terminating.
        ** @return the current status of the future computation. 
        ** @sa ASYNCH_CALL_STATUS
        */
        ASYNCH_CALL_STATUS wait(DWORD dwTimeout) const // Never throws
        {
            return m_pFutureImpl->wait(dwTimeout);
        }

        /*! 
        ** @brief Attempts to abort the computation. If the computation has already started, the call will run till
        **        completion prior to returning ASYNCH_CALL_COMPLETE.
        ** @return the current status of the future computation. 
        ** @sa ASYNCH_CALL_STATUS
        */
        ASYNCH_CALL_STATUS abort() const // May throw
        {
            return m_pFutureImpl->abort();
        }

        /*! 
        ** @brief Waits for the future computation to take place.
        ** @param[in] dwTimeout number of milliseconds to wait before terminating.
        ** @return the value. 
        ** @throw FutureValuePending The value is still being computed, the computation has been aborted, or an internal 
        **        error has occured.
        */
        T getValue() const // May throw
        {
            return m_pFutureImpl->getValue();
        }

    private:
        boost::shared_ptr<Future_Impl<T>> m_pFutureImpl;
        Future& operator=(const Future& other); // Not implemented
    };
}