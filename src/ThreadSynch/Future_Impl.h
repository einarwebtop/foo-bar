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

#include "FutureExceptions.h"

namespace ThreadSynch
{
    /*! 
    ** @brief Describes the status of a future call.
    */
    enum ASYNCH_CALL_STATUS
    {
        /*!
        ** @brief Indicates an error condition
        */
        ASYNCH_CALL_ERROR,

        /*!
        ** @brief Indicates that the computation is still pending
        */
        ASYNCH_CALL_PENDING,

        /*!
        ** @brief Indicates that the computation has completed
        */
        ASYNCH_CALL_COMPLETE,

        /*!
        ** @brief Indicates that the computation has been aborted (successfully)
        */
        ASYNCH_CALL_ABORTED
    };

    /************************************************************************
    ** Generic Future_Impl template
    */

    template<typename T>
    class Future_Impl : private boost::noncopyable
    {
    public:
        typedef boost::function<ASYNCH_CALL_STATUS()> ABORTCALLBACKTYPE;
        typedef boost::function<ASYNCH_CALL_STATUS(DWORD)> WAITCALLBACKTYPE;
        typedef boost::function<T()> GETRETURNVALUECALLBACKTYPE;

        Future_Impl(ABORTCALLBACKTYPE abortCallback,
                    WAITCALLBACKTYPE waitCallback,
                    GETRETURNVALUECALLBACKTYPE getReturnValueCallback)
            : m_abortCallback(abortCallback),
              m_waitCallback(waitCallback),
              m_getReturnValueCallback(getReturnValueCallback)
        {
        }

        ~Future_Impl()
        {
            try
            {
                abort();
            }
            catch(...)
            { /* No exceptions may leave the DTOR */ }
        }

        ASYNCH_CALL_STATUS wait(DWORD dwTimeout) const
        {
            return m_waitCallback(dwTimeout);
        }

        ASYNCH_CALL_STATUS abort() const
        {
            return m_abortCallback();
        }

        T getValue() const
        {
            if(wait(0) != ASYNCH_CALL_COMPLETE)
            {
                throw FutureValuePending();
            }
            return m_getReturnValueCallback();
        }

    private:
        ABORTCALLBACKTYPE m_abortCallback;
        WAITCALLBACKTYPE m_waitCallback;
        GETRETURNVALUECALLBACKTYPE m_getReturnValueCallback;
    };

    /************************************************************************
    ** Future_Impl void specialization
    */

    template<>
    class Future_Impl<void> : private boost::noncopyable
    {
    public:
        typedef boost::function<ASYNCH_CALL_STATUS()> ABORTCALLBACKTYPE;
        typedef boost::function<ASYNCH_CALL_STATUS(DWORD)> WAITCALLBACKTYPE;

        Future_Impl(ABORTCALLBACKTYPE abortCallback,
                    WAITCALLBACKTYPE waitCallback)
            : m_abortCallback(abortCallback),
              m_waitCallback(waitCallback)
        {
        }

        ~Future_Impl()
        {
            try
            {
                abort();
            }
            catch(...)
            { /* No exceptions may leave the DTOR */ }
        }

        ASYNCH_CALL_STATUS wait(DWORD dwTimeout) const
        {
            return m_waitCallback(dwTimeout);
        }

        ASYNCH_CALL_STATUS abort() const
        {
            return m_abortCallback();
        }

        void getValue() const
        {
            if(wait(0) != ASYNCH_CALL_COMPLETE)
            {
                throw FutureValuePending();
            }
            /* void handler: returns nothing */
        }

    private:
        ABORTCALLBACKTYPE m_abortCallback;
        WAITCALLBACKTYPE m_waitCallback;
    };

    /************************************************************************
    ** Future_Impl template partial specializations: Pointer and reference types
    ** are not allowed!
    */ 

    template<typename T>
    class Future_Impl<T*> : private boost::noncopyable
    {
    private:
        Future_Impl();
    };

    template<typename T>
    class Future_Impl<T&> : private boost::noncopyable
    {
    private:
        Future_Impl();
    };
}