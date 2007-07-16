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

#include "FunctorRetvalBinder.h"
#include "ExceptionExpecter.h"

namespace ThreadSynch
{
    /*!@class CallHandler
    ** @brief A class which stores information about a cross thread call.
    ** This class will keep a functor with bound parameters prior to a synchronized call,
    ** and provide a return value and exception information upon completion.
    */
    class CallHandler
    {
    public:
        /************************************************************************
        ** Functions
        */

        /*! Constructor */
        CallHandler();

        /*! Destructor */
        ~CallHandler();

        /*! 
        ** @brief function which sets the callback functor to be run.
        ** @param[in] func 
        **   A functor which accepts no parameters, and returns a 
        **   type T or no value at all (void).
        ** @remarks 
        **   All functor parameters must be bound with boost::bind.
        */
        template<typename T, class E>
        void setCallFunctor(boost::function<T()> func);

        template<class E>
        void setCallFunctor(boost::function<void()> func);

        /*! 
        ** @brief Waits for completion
        ** @param[in] dwTimeout
        **   Time span in milliseconds to wait for completion.
        ** @return Status for the call.
        ** @retval TRUE indicates that the call has completed.
        ** @retval FALSE indicates that the call has not been completed.
        */
        inline BOOL waitForCompletion(DWORD dwTimeout) const
        {
            if(WaitForSingleObject(m_hCompletedEvent, dwTimeout) == WAIT_TIMEOUT)
                return FALSE;
            return TRUE;
        }

        /*! 
        ** @brief Executes the scheduled function.
        ** The scheduled function is executed and the return value is set.
        ** @remarks
        **   When the call completes, an event will be signaled to indicate
        **   that the call has completed. This event will be set regardless
        **   of whether or not the scheduled call throws an exception.
        */
        inline void executeCallback()
        {
            m_executeCall();

            // Notify Thread A that the call has been completed
            SetEvent(m_hCompletedEvent);
        }

        /*!
        ** @brief Returns the status of the scheduled call.
        ** @return Completion status.
        ** @retval TRUE if the call has been made.
        ** @retval FALSE if the call has not yet been made.
        ** @remarks 
        **   Will return TRUE even if an exception was thrown by the call.
        **   Use caughtException() to find whether or not an exception
        **   was caught.
        */
        inline BOOL isCompleted() const
        {
            return (WaitForSingleObject(m_hCompletedEvent, 0) == WAIT_OBJECT_0);
        }
        
        /*!
        ** @return Whether or not the scheduled call threw an exception
        ** @retval TRUE if an exception was thrown and caught
        ** @retval FALSE if no exception was thrown
        */
        inline BOOL caughtException() const
        {
            return m_bExceptionCaught;
        }

        /*! 
        ** @brief Gets the stored return value.
        ** @return The return value as stored by the call.
        */
        template<typename T>
        inline T getReturnValue() const
        {
            return *reinterpret_cast<T*>(m_pReturnValue.get());
        }

        /*! 
        ** @brief Rethrows an exception thrown by the exception expecter
        */
        inline void rethrowException(boost::function<void()> onExceptionDestroyed)
        {
            m_rethrowException(onExceptionDestroyed);
        }

        /*! 
        ** @brief Opens up for locks on the structure.
        ** @return A try_mutex for this structure, which can
        **   either be scoped_try_lock'ed or scoped_lock'ed.
        */
        inline boost::try_mutex* getAccessMutex() const
        {
            return &m_accessMutex;
        }

    private:
        /************************************************************************
        ** Variables
        */

        /*! 
        ** An inner event handle which signals completion.
        */
        HANDLE m_hCompletedEvent;

        /*! 
        ** Memory block which receives and stores the return value of the scheduled function.
        */
        boost::scoped_array<BYTE> m_pReturnValue;
        
        /*! 
        ** A functor which executes the call and attempts to retrieve the return value.
        */
        boost::function<void()> m_executeCall;

        /*! 
        ** A functor which rethrows exceptions picked up by the expecter.
        ** The functor accepts one parameter, which is another functor to be
        ** called as a thrown exception object is destroyed.
        */
        boost::function<void(boost::function<void()>)> m_rethrowException;

        /*! 
        ** A functor which instructs the retval binder class to free itself -- needed 
        ** since we cannot reference the class without full template parameter information.
        */
        boost::function<void()> m_freeRetvalBinder;

        /*! 
        ** A functor which instructs the exception expecter class to free itself -- needed 
        ** since we cannot reference the class without full template parameter information.
        */
        boost::function<void()> m_freeExceptionExpecter;

        /*! 
        ** Inner mutex which provides locking functionality on the structure.
        */
        mutable boost::try_mutex m_accessMutex;

        /*! 
        ** Indicates whether or not setCallFunctor has been called already
        */
        BOOL m_bCallFunctorSet;

        /*!
        ** Indicates whether or not an exception was thrown by the call
        */
        BOOL m_bExceptionCaught;

        /*!
        ** @brief notification that the scheduled call handled threw an exception
        ** @param[in] etype Specifies which exception, if any, was thrown
        ** @remark
        **   Called by the exception expecter when a call has been completed,
        **   in context of the thread that does the pickup.
        */
        void onExceptionExpecterComplete(details::CaughtExceptionType etype);
    };

    /************************************************************************
    ** Implementation of non-inline template member functions for CallHandler
    */

    CallHandler::CallHandler()
        : m_bCallFunctorSet(FALSE),
          m_bExceptionCaught(FALSE)
    {
        m_hCompletedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    CallHandler::~CallHandler()
    {
        CloseHandle(m_hCompletedEvent);

        // Deallocate retval binder, if one is set
        if(m_freeRetvalBinder)
        {
            m_freeRetvalBinder();
        }

        // Deallocate the exeception expecter, if one is set
        if(m_freeExceptionExpecter)
        {
          m_freeExceptionExpecter();
        }
    }

    template<typename T, class E>
    void CallHandler::setCallFunctor(boost::function<T()> func)
    {
        // Disallow multiple runs
        if(m_bCallFunctorSet)
        {
            throw std::exception("Callback already set");
        }
        m_bCallFunctorSet = TRUE;

        m_pReturnValue.reset(new BYTE[sizeof(T)]);
        
        // Create a loose ExceptionExpecter instance. The instance will be free'd through the
        // m_freeExceptionExpecter functor, called in the CallHandler destructor.
        ExceptionExpecter<E>* expecter = new ExceptionExpecter<E>(boost::bind(&CallHandler::onExceptionExpecterComplete, this, _1));
        m_rethrowException = boost::bind(&ExceptionExpecter<E>::rethrow, expecter, _1);
        m_freeExceptionExpecter = boost::bind(&ExceptionExpecter<E>::free, expecter);

        // Create a loose FunctorRetvalBinder instance. The instance will be free'd through the
        // m_freeRetvalBinder functor, called in the CallHandler destructor.
        FunctorRetvalBinder<T>* binder = new FunctorRetvalBinder<T>(func, m_pReturnValue.get());
        m_freeRetvalBinder = boost::bind(&FunctorRetvalBinder<T>::free, binder);

        // Setup the main execution functor, which will wrap both expected exceptions and
        // return value.
        m_executeCall = boost::bind(&ExceptionExpecter<E>::execute, 
                                    expecter, 
                                    static_cast<boost::function<void()>>(boost::bind(&FunctorRetvalBinder<T>::execute, binder)));
    }

    template<class E>
    void CallHandler::setCallFunctor(boost::function<void()> func)
    {
        // Disallow multiple runs
        if(m_bCallFunctorSet)
        {
            throw std::exception("Callback already set");
        }
        m_bCallFunctorSet = TRUE;

        // Create a loose ExceptionExpecter instance. The instance will be free'd through the
        // m_freeExceptionExpecter functor, called in the CallHandler destructor.
        ExceptionExpecter<E>* expecter = new ExceptionExpecter<E>(boost::bind(&CallHandler::onExceptionExpecterComplete, this, _1));
        m_rethrowException = boost::bind(&ExceptionExpecter<E>::rethrow, expecter, _1);
        m_freeExceptionExpecter = boost::bind(&ExceptionExpecter<E>::free, expecter);

        // Create a loose FunctorRetvalBinder instance. The instance will be free'd through the
        // m_freeRetvalBinder functor, called in the CallHandler destructor.
        FunctorRetvalBinder<void>* binder = new FunctorRetvalBinder<void>(func, m_pReturnValue.get());
        m_freeRetvalBinder = boost::bind(&FunctorRetvalBinder<void>::free, binder);

        // Setup the main execution functor, which will wrap both expected exceptions and
        // return value.
        m_executeCall = boost::bind(&ExceptionExpecter<E>::execute, 
                                    expecter, 
                                    static_cast<boost::function<void()>>(boost::bind(&FunctorRetvalBinder<void>::execute, binder)));
    }

    void CallHandler::onExceptionExpecterComplete(details::CaughtExceptionType etype)
    {
        if(etype != details::CaughtExceptionType_None)
        {
            m_bExceptionCaught = TRUE;
        }
    }
}