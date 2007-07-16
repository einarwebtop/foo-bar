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

#include "CallHandler.h"
#include "PickupPolicyProvider.h"
#include "CallSchedulerExceptions.h"

namespace ThreadSynch
{
    #define ExceptionTypes boost::mpl::vector

    /*!@class CallScheduler
    ** @brief A singleton class which enables a user to schedule calls across threads
    ** The PickupPolicy template parameter will decide how notifications are transported
    ** between the threads.
    ** @remark
    **   Todo:
    **     - Possible tune-up: 
    **       Keep one lock per queue, in addition to one for all queues. 
    **       The call structure may become slow in case of many
    **       simultaneous and quick routines being called cross threads, in which
    **       case the queue mutex will lock and unlock quite rapidly.
    **     - Deal with TLS return values and parameters, if possible.
    */
    template<class PickupPolicy>
    class CallScheduler
    {
    public:
        /************************************************************************
        ** Functions
        */ 

        /*! 
        ** @return A pointer to the CallScheduler implementation.
        **/
        static CallScheduler* getInstance();

        /*! 
        ** @brief The function which schedules calls to be made across threads, and expects a few exceptions might be thrown.
        ** @warning Don't specify "void" for ReturnValueType. Call syncCall() or syncCall<ExceptionTypes<>> instead.
        ** @param[in] dwThreadId the id of the thread to make the call in.
        ** @param[in] callback functor which executes the callback.
        ** @param[in] dwTimeout number of milliseconds to wait before terminating.
        ** @param[in] ReturnValueType return value, usually deduced from the boost function.
        ** @param[in] Exceptions expected exceptions, specified as a comma separated template parameters to ExceptionTypes.
        ** @return the returned value from the synchronized call.
        */
        template<typename ReturnValueType, class Exceptions>
        ReturnValueType syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout);

        /*! 
        ** @brief The function which schedules calls to be made across threads.
        ** @warning Don't specify "void" for ReturnValueType. Call syncCall() or syncCall<ExceptionTypes<>> instead.
        ** @param[in] dwThreadId the id of the thread to make the call in.
        ** @param[in] callback functor which executes the callback.
        ** @param[in] dwTimeout number of milliseconds to wait before terminating.
        ** @param[in] ReturnValueType return value, usually deduced from the boost function.
        ** @return the returned value from the synchronized call.
        */
        template<typename ReturnValueType>
        ReturnValueType syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout);

        /*! 
        ** @brief The function which schedules calls to be made across threads, and expects a few exceptions might be thrown.
        ** @param[in] dwThreadId the id of the thread to make the call in.
        ** @param[in] callback functor which executes the callback.
        ** @param[in] dwTimeout number of milliseconds to wait before terminating.
        ** @param[in] Exceptions expected exceptions, specified as a comma separated template parameters to ExceptionTypes.
        ** @note This function serves synchronized calls with no (void) return value.
        */
        template<class Exceptions>
        void syncCall(DWORD dwThreadId, boost::function<void()> callback, DWORD dwTimeout);

        /*! 
        ** @brief The function which schedules calls to be made across threads.
        ** @param[in] dwThreadId the id of the thread to make the call in.
        ** @param[in] callback functor which executes the callback.
        ** @param[in] dwTimeout number of milliseconds to wait before terminating.
        ** @note This function serves synchronized calls with no (void) return value.
        */
        void syncCall(DWORD dwThreadId, boost::function<void()> callback, DWORD dwTimeout);

        /*! 
        ** @brief Exceutes all scheuled calls for the current thread.
        ** @param[in] pSchedulerInstance which singleton instance to run the operations on.
        */
        static void APIENTRY executeScheduledCalls(CallScheduler* pSchedulerInstance);

    private:
        /************************************************************************
        ** Types
        */ 
        
        typedef std::list<CallHandler*> CALLQUEUE;
        typedef std::map<DWORD, CALLQUEUE> THREADCALLQUEUE;

        /************************************************************************
        ** Variables
        */ 

        static CallScheduler* m_pInstance;
        static boost::mutex m_instanceMutex;
        
        // CallHandlers will not be deleted, and the queue will not be 
        // affected outside a lock on this mutex
        boost::mutex m_threadQueueMutex;
        THREADCALLQUEUE m_threadQueue;

        /************************************************************************
        ** Functions 
        */
        
        /*! 
        ** @brief Constructor
        */
        CallScheduler();

        /*! 
        ** @brief Internal helper function shared between the different syncCall flavors
        */
        void executeCallHandler(DWORD dwThreadId, CallHandler* pCallHandler, DWORD dwTimeout, boost::shared_ptr<boost::try_mutex::scoped_try_lock> pCallHandlerLock);

        /*! 
        ** @brief Copy constructor
        ** @remarks Not implemented, to avoid copies.
        */
        CallScheduler(const CallScheduler&); // not implemented

        /*! 
        ** @brief Assignment operator 
        ** @remarks Not implemented, to avoid copies.
        */
        const CallScheduler& operator =(const CallScheduler&); // not implemented

        /*! 
        ** @brief adds a call to the specified therad's queue.
        ** @param[in] dwThreadId the id of the thread to enqueue in.
        ** @param[in] pCallback pointer to a CallHandler instance in which the details of the callback functor resides.
        */
        void enqueueThreadCall(DWORD dwThreadId, CallHandler* pCallback);

        /*! 
        ** @brief removes a call off a thread's queue.
        ** @param[in] dwThreadId the id of the thread to enqueue in.
        ** @param[in] pCallback pointer to a CallHandler instance in which the details of the callback functor resides.
        */
        void dequeueThreadCall(DWORD dwThreadId, CallHandler* pCallback);

        /*! 
        ** @brief Function to fetch the next CallHandler off the specified queue.
        ** @param[in] dwThreadId the id of the thread to get a call for.
        ** @param[in] pCallHandlerLock a lock object, which has locked a resource in the CallHandler for simultaneous access.
        ** @return The next scheduled CallHandler.
        */
        CallHandler* getNextCallFromQueue(DWORD dwThreadId, boost::shared_ptr<boost::try_mutex::scoped_try_lock> pCallHandlerLock);

        /*!
        ** @brief Callback for CallHandler's rethrow mechanism
        ** @remark
        **   Will be called by the throwHooked wrapper when a re-thrown exception has been destroyed.
        **   The shared_ptr will take care of deleting the CallHandler pointer.
        */    
        static void onRethrownExceptionDestroyed(boost::shared_ptr<CallHandler> pCallHandler)
        { /* No actions */ }
    };

    /************************************************************************
    ** Static variable definitions
    */

    template<class PickupPolicy> CallScheduler<PickupPolicy>* CallScheduler<PickupPolicy>::m_pInstance = NULL;
    template<class PickupPolicy> boost::mutex CallScheduler<PickupPolicy>::m_instanceMutex;

    /************************************************************************
    ** Implementation of non-inline template member functions
    */

    template<class PickupPolicy>
    CallScheduler<PickupPolicy>* CallScheduler<PickupPolicy>::getInstance()
    {
        boost::mutex::scoped_lock lock(m_instanceMutex);
        if(m_pInstance == NULL)
        {
            m_pInstance = new CallScheduler();
        }
        return m_pInstance;
    }

    template<class PickupPolicy>
    template<typename ReturnValueType, typename Exceptions>
    ReturnValueType CallScheduler<PickupPolicy>::syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout)
    {
        boost::shared_ptr<CallHandler> pCallHandler(new CallHandler());

        // Initialize the container which holds the call to be done by the target thread
        pCallHandler->setCallFunctor<ReturnValueType, Exceptions>(callback);

        boost::shared_ptr<boost::try_mutex::scoped_try_lock> pCallHandlerLock;
        // Process the call handler
        executeCallHandler(dwThreadId, pCallHandler.get(), dwTimeout, pCallHandlerLock);

        BOOL bRetvalSet = FALSE;
        ReturnValueType returnValue;

        // Check if the call completed, and if yes; store value.
        if(pCallHandler->isCompleted())
        {
            if(pCallHandler->caughtException())
            {
                // Rethrow caught exceptions. This also leaves control of the pCallHandler pointer in the hands
                // of the rethrow mechanism, and specifically onRethrownExceptionDestroyed, which will be called
                // once the exception has been caught and destroyed.
                pCallHandler->rethrowException(boost::bind(onRethrownExceptionDestroyed, pCallHandler));
            }
            else
            {
                returnValue = pCallHandler->getReturnValue<ReturnValueType>();
                bRetvalSet = TRUE;
            }
        }
        else
        {
            // Call function to lock queue (while callhandler is also locked) and then de-queue
            dequeueThreadCall(dwThreadId, pCallHandler.get());
        }

        if(bRetvalSet)
        {
// If we get here, the return value _is_ set, so ignore the warning.
#pragma warning(push)
#pragma warning(disable: 4701)
            return returnValue;
#pragma warning(pop)
        }
        else
        {
            throw CallTimeoutException();
        }
    }

    template<class PickupPolicy>
    template<typename ReturnValueType>
    ReturnValueType CallScheduler<PickupPolicy>::syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout)
    {
        return syncCall<ReturnValueType, boost::mpl::vector<>>(dwThreadId, callback, dwTimeout);
    }

    template<class PickupPolicy>
    template<typename Exceptions>
    void CallScheduler<PickupPolicy>::syncCall(DWORD dwThreadId, boost::function<void()> callback, DWORD dwTimeout)
    {
        boost::shared_ptr<CallHandler> pCallHandler(new CallHandler());

        // Initialize the container which holds the call to be done by the target thread
        pCallHandler->setCallFunctor<Exceptions>(callback);

        boost::shared_ptr<boost::try_mutex::scoped_try_lock> pCallHandlerLock;
        // Process the call handler
        executeCallHandler(dwThreadId, pCallHandler.get(), dwTimeout, pCallHandlerLock);

        BOOL bRetvalSet = FALSE;

        // Check if the call completed, and if yes; store value.
        if(pCallHandler->isCompleted())
        {
            if(pCallHandler->caughtException())
            {
                // Rethrow caught exceptions. This also leaves control of the pCallHandler pointer in the hands
                // of the rethrow mechanism, and specifically onRethrownExceptionDestroyed, which will be called
                // once the exception has been caught and destroyed.
                pCallHandler->rethrowException(boost::bind(onRethrownExceptionDestroyed, pCallHandler));
            }
            else
            {
                bRetvalSet = TRUE;
            }
        }
        else
        {
            // Call function to lock queue (while callhandler is also locked) and then de-queue
            dequeueThreadCall(dwThreadId, pCallHandler.get());
        }

        if(bRetvalSet)
        {
            return;
        }
        else
        {
            throw CallTimeoutException();
        }
    }

    template<class PickupPolicy>
    void CallScheduler<PickupPolicy>::syncCall(DWORD dwThreadId, boost::function<void()> callback, DWORD dwTimeout)
    {
        syncCall<boost::mpl::vector<>>(dwThreadId, callback, dwTimeout);
    }

    template<class PickupPolicy>
    CallScheduler<PickupPolicy>::CallScheduler()
    {
    }

    template<class PickupPolicy>
    void CallScheduler<PickupPolicy>::executeCallHandler(DWORD dwThreadId, CallHandler* pCallHandler, DWORD dwTimeout, boost::shared_ptr<boost::try_mutex::scoped_try_lock> pCallHandlerLock)
    {
        try
        {
            // Enqueue the call and notify the pickup policy
            enqueueThreadCall(dwThreadId, pCallHandler);
        }
        catch(CallSchedulingFailedException&)
        {
            throw;
        }

        pCallHandler->waitForCompletion(dwTimeout);

        // Before pCallHandler is deleted, a lock on both the handler mutex and the queue mutex must be in place
        // only upon the two in place can the handler be deleted

        // Obtain a lock on the access handler, preventing the scheduler to open it if the call
        // has not yet been begun. If the callback sequence has been set in motion, this lock will
        // wait until it has completed.
        pCallHandlerLock.reset(new boost::try_mutex::scoped_try_lock(*pCallHandler->getAccessMutex()));
    }

    template<class PickupPolicy>
    void CallScheduler<PickupPolicy>::enqueueThreadCall(DWORD dwThreadId, CallHandler* pCallback)
    {
        // Acquire a lock on the thread queue
        boost::mutex::scoped_lock lock(m_threadQueueMutex);

        // If there's no previously scheduled calls for that queue, we've got a schedule a pickup now
        BOOL bMustQueueThreadPickup = (m_threadQueue.find(dwThreadId) == m_threadQueue.end());

        // Put the call onto the queue of calls waiting to happen
        m_threadQueue[dwThreadId].push_back(pCallback);

        if(bMustQueueThreadPickup)
        {
            try
            {
                PickupPolicy::scheduleThreadCallback(dwThreadId, 
                                                     reinterpret_cast<PickupPolicyProvider::PCALLBACK>(&CallScheduler::executeScheduledCalls), 
                                                     reinterpret_cast<ULONG_PTR>(this));
            }
            catch(PickupSchedulingFailedException&)
            {
                // Cleanup the thread queue. If this is left in place, there will be dead pointers there.
                m_threadQueue.erase(dwThreadId);

                // Todo: update the message thrown to something reported by the policy
                throw CallSchedulingFailedException("PickupPolicyProvider reported a failure");
            }
            catch(...)
            {
                // Cleanup the thread queue. If this is left in place, there will be dead pointers there.
                m_threadQueue.erase(dwThreadId);

                // Catch all other exceptions and throw a generic message
                throw CallSchedulingFailedException("PickupPolicyProvider reported a failure");
            }
        }
    }

    template<class PickupPolicy>
    void CallScheduler<PickupPolicy>::dequeueThreadCall(DWORD dwThreadId, CallHandler* pCallback)
    {
        boost::mutex::scoped_lock lock(m_threadQueueMutex);
        
        THREADCALLQUEUE::iterator threadQueueIter = m_threadQueue.find(dwThreadId);
        if(threadQueueIter == m_threadQueue.end())
        {
            // No queue for that thread id, so bail.
            return;
        }

        CALLQUEUE::iterator callQueueIter = std::find((*threadQueueIter).second.begin(), (*threadQueueIter).second.end(), pCallback);
        if(callQueueIter == (*threadQueueIter).second.end())
        {
            // The callback was not found in the thread's queue
            return;
        }
        
        (*threadQueueIter).second.erase(callQueueIter);
    }

    template<class PickupPolicy>
    CallHandler* CallScheduler<PickupPolicy>::getNextCallFromQueue(DWORD dwThreadId, 
                                                                   boost::shared_ptr<boost::try_mutex::scoped_try_lock> pCallHandlerLock)
    {
		// Acquire a lock on the thread queue
		boost::mutex::scoped_lock lock(m_threadQueueMutex);
		THREADCALLQUEUE::iterator threadQueueIterator;

		if((threadQueueIterator = m_threadQueue.find(dwThreadId)) != m_threadQueue.end())
		{
			// Find the queue for the current requested thread
			CALLQUEUE* pCallbackQueue = &((*threadQueueIterator).second);

            CALLQUEUE::iterator callbackQueueIterator;
            for(callbackQueueIterator = pCallbackQueue->begin(); callbackQueueIterator != pCallbackQueue->end(); ++ callbackQueueIterator)
			{
                CallHandler* pCallHandler = *callbackQueueIterator;
				
				// Try to acquire a lock on the call handler, to prevent the scheduler from deallocating it
				// while the function is running. While this lock is in place, the syncCall cannot
				// destroy the CallHandler.
				pCallHandlerLock.reset(new boost::try_mutex::scoped_try_lock(*pCallHandler->getAccessMutex(), false));
                if(!pCallHandlerLock->try_lock())
                {
                    // We didn't get a lock on the CallHandler. This means that it's taken, and should not be parsed at this time.
                    // Continue with the next queued item.
                    pCallHandlerLock.reset();
                    continue;
                }

                // The lock was obtained, so we can pop it off the queue
				pCallbackQueue->pop_front();
				
				// If this was the last item in the queues thread ..
				if(pCallbackQueue->empty())
				{
					// Erase the thread's queue
					m_threadQueue.erase(threadQueueIterator);
				}

				return pCallHandler;
			}
		}
		return NULL;
    }

    template<class PickupPolicy>
    void APIENTRY CallScheduler<PickupPolicy>::executeScheduledCalls(CallScheduler* pSchedulerInstance)
    {
        DWORD dwThreadId = GetCurrentThreadId();
        CallHandler* pCallHandler;
        boost::shared_ptr<boost::try_mutex::scoped_try_lock> pCallHandlerLock;

        while((pCallHandler = pSchedulerInstance->getNextCallFromQueue(dwThreadId, pCallHandlerLock)) != NULL)
        {
            // A call handler has been checked out of the structure
            pCallHandler->executeCallback();

            // Once this lock is reset, pCallHandler isn't guaranteed to be valid anymore
            pCallHandlerLock.reset();
        }
    }
}