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

#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include "CallHandler.h"
#include "PickupPolicyProvider.h"
#include "CallSchedulerExceptions.h"
#include "Future.h"

namespace ThreadSynch
{
    /************************************************************************
    ** Definitions
    */
	
    #define ExceptionTypes boost::mpl::vector

    #define IS_VOID_OR_SEQUENCE(x)\
        boost::mpl::or_                 \
        <                               \
            boost::is_void<x>,          \
            boost::mpl::is_sequence<x>  \
        > /*****************************/

    #define X_IS_VOID_AND_Y_IS_SEQUENCE(x, y)\
        boost::mpl::and_                \
        <                               \
            boost::is_void<x>,          \
            boost::mpl::is_sequence<y>  \
        > /*****************************/

    #define X_IS_NON_VOID_AND_Y_IS_SEQUENCE(x, y)\
        boost::mpl::and_                         \
        <                                        \
            boost::mpl::not_<boost::is_void<x>>, \
            boost::mpl::is_sequence<y>           \
        > /**************************************/

	/*!@class CallScheduler
	** @brief A singleton class which enables a user to schedule calls across threads
	** The PickupPolicy template parameter will decide how notifications are transported
	** between the threads.
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
		** @brief schedules calls to be made across threads, and expects a few exceptions might be thrown.
		** @param[in] dwThreadId the id of the thread to make the call in.
		** @param[in] callback functor which executes the callback.
		** @param[in] dwTimeout number of milliseconds to wait before terminating.
		** @param[in] ReturnValueType return value, usually deduced, but specify to avoid possibly cryptic errors.
		** @param[in] Exceptions expected exceptions, specified as a comma separated template parameters to ExceptionTypes.
		** @return the returned value from the synchronized call.
		*/
        template<typename ReturnValueType, class Exceptions>
        typename boost::disable_if<IS_VOID_OR_SEQUENCE(ReturnValueType), ReturnValueType>::
        type syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout);

		/*! 
		** @brief schedules calls to be made across threads, and expects a few exceptions might be thrown.
		** @param[in] dwThreadId the id of the thread to make the call in.
		** @param[in] callback functor which executes the callback.
		** @param[in] dwTimeout number of milliseconds to wait before terminating.
        ** @param[in] ReturnValueType return value, usually deduced, but specify to avoid possibly cryptic errors.
        ** @param[in] Exceptions expected exceptions, specified as a comma separated template parameters to ExceptionTypes.
		*/
        template<typename ReturnValueType, class Exceptions>
        typename boost::enable_if<boost::is_void<ReturnValueType>, ReturnValueType>::
        type syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout);

#pragma region syncCall template parameter redirections
        // ReturnValueType IS NOT void AND ReturnValueType IS NOT MPL Sequence redirection
        template<typename ReturnValueType>
        typename boost::disable_if<IS_VOID_OR_SEQUENCE(ReturnValueType), ReturnValueType>::
        type syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout)
        {
            return syncCall<ReturnValueType, ExceptionTypes<>>(dwThreadId, callback, dwTimeout);
        }

        // ReturnValueType IS void redirection
        template<typename ReturnValueType>
        typename boost::enable_if<boost::is_void<ReturnValueType>, ReturnValueType>::
        type syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout)
        {
            syncCall<ReturnValueType, ExceptionTypes<>>(dwThreadId, callback, dwTimeout);
        }

        // ReturnValueType IS NOT void AND Exceptions IS Sequence redirection
        template<typename Exceptions, typename ReturnValueType>
        typename boost::enable_if<X_IS_NON_VOID_AND_Y_IS_SEQUENCE(ReturnValueType, Exceptions), ReturnValueType>::
            type syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout)
        {
            return syncCall<ReturnValueType, Exceptions>(dwThreadId, callback, dwTimeout);
        }

        // ReturnValueType IS void AND Exceptions IS Sequence redirection
        template<typename Exceptions, typename ReturnValueType>
        typename boost::enable_if<X_IS_VOID_AND_Y_IS_SEQUENCE(ReturnValueType, Exceptions), ReturnValueType>::
        type syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout)
        {
            syncCall<ReturnValueType, Exceptions>(dwThreadId, callback, dwTimeout);
        }
#pragma endregion

        /*! 
        ** @brief schedules calls to be made across threads, and expects a few exceptions might be thrown.
        ** @param[in] dwThreadId the id of the thread to make the call in.
        ** @param[in] callback functor which executes the callback.
        ** @param[in] ReturnValueType return value type. This template overload handles non-void types.
        ** @param[in] Exceptions expected exceptions, specified as a comma separated template parameters to ExceptionTypes.
        ** @return a Future-object which will hold the result of the async call, and also enables early aborts / waits.
        ** @sa Future
        ** @throw std::bad_alloc if the Future object cannot be created
        */
        template<typename ReturnValueType, class Exceptions>
        typename boost::disable_if<IS_VOID_OR_SEQUENCE(ReturnValueType), Future<ReturnValueType>>::
        type asyncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback);

        /*! 
        ** @brief schedules calls to be made across threads, and expects a few exceptions might be thrown.
        ** @param[in] dwThreadId the id of the thread to make the call in.
        ** @param[in] callback functor which executes the callback.
        ** @param[in] ReturnValueType return value type. This template overload handles the void type only.
        ** @param[in] Exceptions expected exceptions, specified as a comma separated template parameters to ExceptionTypes.
        ** @return a Future-object which will hold the result of the async call, and also enables early aborts / waits.
        ** @sa Future
        ** @throw std::bad_alloc if the Future object cannot be created
        */
        template<typename ReturnValueType, class Exceptions>
        typename boost::enable_if<boost::is_void<ReturnValueType>, Future<ReturnValueType>>::
        type asyncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback);

#pragma region asyncCall template parameter redirections
        // ReturnValueType IS NOT MPL Sequence redirection
        template<typename ReturnValueType>
        typename boost::disable_if<boost::mpl::is_sequence<ReturnValueType>, Future<ReturnValueType>>::
        type asyncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback)
        {
            return asyncCall<ReturnValueType, ExceptionTypes<>>(dwThreadId, callback);
        }

        // Exceptions IS MPL Sequence redirection
        template<typename Exceptions, typename ReturnValueType>
        typename boost::enable_if<boost::mpl::is_sequence<Exceptions>, Future<ReturnValueType>>::
        type asyncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback)
        {
            return asyncCall<ReturnValueType, Exceptions>(dwThreadId, callback);
        }
#pragma endregion

		/*! 
		** @brief Executes all scheduled calls for the current thread.
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
        ** @brief callback for asynchronous Future objects, which aborts a scheduled call
        ** @param[in] dwThreadId the id of the thread the Asynchronous call will execute on
        ** @param[in] pCallHandler smart pointer to a CallHandler
        ** @remark If the call has already begun, this function will wait for it to end.
        ** @throw ... Any exceptions thrown during the execution of a started call will be thrown.
        */
        ASYNCH_CALL_STATUS abortAsyncCall(DWORD dwThreadId, boost::shared_ptr<CallHandler> pCallHandler);
        
        /*!
        ** @brief callback for asynchronous Future objects, which causes the thread to wait for the started call to complete.
        ** @param[in] pCallHandler smart pointer to a CallHandler
        ** @param[in] dwTimeout the number of milliseconds to wait for the call to complete. Specify INFINITE to wait without timeouts.
        */
        ASYNCH_CALL_STATUS waitAsyncCall(boost::shared_ptr<CallHandler> pCallHandler, DWORD dwTimeout);

		/*! 
		** @brief adds a call to the specified therad's queue.
		** @param[in] dwThreadId the id of the thread to enqueue in.
		** @param[in] pCallHandler pointer to a CallHandler instance in which the details of the callback functor resides.
		*/
		void enqueueThreadCall(DWORD dwThreadId, CallHandler* pCallHandler);

		/*! 
		** @brief removes a call off a thread's queue.
		** @param[in] dwThreadId the id of the thread to enqueue in.
		** @param[in] pCallHandler pointer to a CallHandler instance in which the details of the callback functor resides.
		*/
		void dequeueThreadCall(DWORD dwThreadId, CallHandler* pCallHandler);

		/*! 
		** @brief Function to fetch the next CallHandler off the specified queue.
		** @param[in] dwThreadId the id of the thread to get a call for.
		** @param[in] pCallHandlerLock a lock object, which has locked a resource in the CallHandler for simultaneous access.
		** @return The next scheduled CallHandler.
		*/
        CallHandler* getNextCallFromQueue(DWORD dwThreadId, boost::scoped_ptr<boost::try_mutex::scoped_try_lock>& pCallHandlerLock);

        /*!
		** @brief Callback for CallHandler's rethrow mechanism
		** @remark
		**   Will be called by the throwHooked wrapper when a re-thrown exception has been destroyed.
		**   The boost::shared_ptr will take care of deleting the CallHandler pointer.
		*/    
		static void onRethrownExceptionDestroyed(boost::shared_ptr<CallHandler> pCallHandler)
		{ /* No actions */ }

        /*! 
        ** @brief Internal helper function shared between the different syncCall flavors
        */
        void processSynchronousCallHandler(DWORD dwThreadId, CallHandler* pCallHandler, DWORD dwTimeout, boost::scoped_ptr<boost::try_mutex::scoped_lock>& pCallHandlerLock);

        /*! 
        ** @brief Internal helper function shared between the different asyncCall flavors
        */
        void preProcessAsynchronousCallHandler(DWORD dwThreadId, CallHandler* pCallHandler);
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

#pragma warning(push)
#pragma warning(disable: 4715)
    template<class PickupPolicy>
    template<typename ReturnValueType, class Exceptions>
    typename boost::disable_if<IS_VOID_OR_SEQUENCE(ReturnValueType), ReturnValueType>::
    type CallScheduler<PickupPolicy>::syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout)
    {
		boost::shared_ptr<CallHandler> pCallHandler(new CallHandler());

		// Initialize the container which holds the call to be done by the target thread
		pCallHandler->setCallFunctor<ReturnValueType, Exceptions>(callback);

        boost::scoped_ptr<boost::try_mutex::scoped_lock> pCallHandlerLock;
		// Process the call handler, and add it to the queue
		processSynchronousCallHandler(dwThreadId, pCallHandler.get(), dwTimeout, pCallHandlerLock);

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
				return pCallHandler->getReturnValue<ReturnValueType>();
			}
		}
		else
		{
			// Call function to lock queue (while callhandler is also locked) and then de-queue
			dequeueThreadCall(dwThreadId, pCallHandler.get());
            throw CallTimeoutException();
		}
	}
#pragma warning(pop)

    template<class PickupPolicy>
    template<typename ReturnValueType, class Exceptions>
    typename boost::enable_if<boost::is_void<ReturnValueType>, ReturnValueType>::
    type CallScheduler<PickupPolicy>::syncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback, DWORD dwTimeout)
	{
		boost::shared_ptr<CallHandler> pCallHandler(new CallHandler());

		// Initialize the container which holds the call to be done by the target thread
		pCallHandler->setCallFunctor<ReturnValueType, Exceptions>(callback);

        boost::scoped_ptr<boost::try_mutex::scoped_lock> pCallHandlerLock;
		// Process the call handler, and add it to the queue
		processSynchronousCallHandler(dwThreadId, pCallHandler.get(), dwTimeout, pCallHandlerLock);

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
                return;
            }
        }
        else
        {
            // Call function to lock queue (while callhandler is also locked) and then de-queue
            dequeueThreadCall(dwThreadId, pCallHandler.get());
            throw CallTimeoutException();
        }
	}

    template<class PickupPolicy>
    template<typename ReturnValueType, class Exceptions>
    typename boost::disable_if<IS_VOID_OR_SEQUENCE(ReturnValueType), Future<ReturnValueType>>::
    type CallScheduler<PickupPolicy>::asyncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback)
    {
        boost::shared_ptr<CallHandler> pCallHandler(new CallHandler());

        // The Future construction can possibly throw (specifically a std::bad_alloc), so the construction must be done prior to
        // the CallHandler being added to the queue. Should an exception be thrown, pCallHandler will clean itself, and nothing
        // else will have gone wrong.
        Future<ReturnValueType> futureObject = Future<ReturnValueType>(boost::bind(&CallScheduler<PickupPolicy>::abortAsyncCall, this, dwThreadId, pCallHandler),
                                                                       boost::bind(&CallScheduler<PickupPolicy>::waitAsyncCall, this, pCallHandler, _1), 
                                                                       boost::bind(&CallHandler::getReturnValue<ReturnValueType>, pCallHandler.get()));

        // Initialize the container which holds the call to be done by the target thread
        pCallHandler->setCallFunctor<ReturnValueType, Exceptions>(callback);

        // Add the handler to the queue
        preProcessAsynchronousCallHandler(dwThreadId, pCallHandler.get());

        return futureObject;
    }

    template<class PickupPolicy>
    template<typename ReturnValueType, class Exceptions>
    typename boost::enable_if<boost::is_void<ReturnValueType>, Future<ReturnValueType>>::
    type CallScheduler<PickupPolicy>::asyncCall(DWORD dwThreadId, boost::function<ReturnValueType()> callback)
    {
        boost::shared_ptr<CallHandler> pCallHandler(new CallHandler());

        // The Future construction can possibly throw (specifically a std::bad_alloc), so the construction must be done prior to
        // the CallHandler being added to the queue. Should an exception be thrown, pCallHandler will clean itself, and nothing
        // else will have gone wrong.
        Future<ReturnValueType> futureObject = Future<ReturnValueType>(boost::bind(&CallScheduler<PickupPolicy>::abortAsyncCall, this, dwThreadId, pCallHandler),
                                                                       boost::bind(&CallScheduler<PickupPolicy>::waitAsyncCall, this, pCallHandler, _1));

        // Initialize the container which holds the call to be done by the target thread
        pCallHandler->setCallFunctor<ReturnValueType, Exceptions>(callback);

        // Add the handler to the queue
        preProcessAsynchronousCallHandler(dwThreadId, pCallHandler.get());

        return futureObject;
    }

    template<class PickupPolicy>
	CallScheduler<PickupPolicy>::CallScheduler()
	{
        /* Empty CTOR */
	}

#pragma warning(push)
#pragma warning(disable: 4715)
    template<class PickupPolicy>
    ASYNCH_CALL_STATUS CallScheduler<PickupPolicy>::abortAsyncCall(DWORD dwThreadId, boost::shared_ptr<CallHandler> pCallHandler)
    {
        boost::scoped_ptr<boost::try_mutex::scoped_lock> pCallHandlerLock;
        // Attempt to obtain a lock on the CallHandler
        pCallHandlerLock.reset(new boost::try_mutex::scoped_lock(*pCallHandler->getAccessMutex()));

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
                return ASYNCH_CALL_COMPLETE;
            }
        }
        else
        {
            // Call function to lock queue (while callhandler is also locked) and then de-queue
            dequeueThreadCall(dwThreadId, pCallHandler.get());
            return ASYNCH_CALL_ABORTED;
        }
    }
#pragma warning(pop)

    template<class PickupPolicy>
    ASYNCH_CALL_STATUS CallScheduler<PickupPolicy>::waitAsyncCall(boost::shared_ptr<CallHandler> pCallHandler, DWORD dwTimeout)
    {
        if(pCallHandler->waitForCompletion(dwTimeout))
        {
            return ASYNCH_CALL_COMPLETE;
        }
        else
        {
            return ASYNCH_CALL_PENDING;
        }
    }

	template<class PickupPolicy>
	void CallScheduler<PickupPolicy>::enqueueThreadCall(DWORD dwThreadId, CallHandler* pCallHandler)
	{
		// Acquire a lock on the thread queue
		boost::mutex::scoped_lock lock(m_threadQueueMutex);

		// If there's no previously scheduled calls for that queue, we've got a schedule a pickup now
		BOOL bMustQueueThreadPickup = (m_threadQueue.find(dwThreadId) == m_threadQueue.end());

		// Put the call onto the queue of calls waiting to happen
		m_threadQueue[dwThreadId].push_back(pCallHandler);

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
	void CallScheduler<PickupPolicy>::dequeueThreadCall(DWORD dwThreadId, CallHandler* pCallHandler)
	{
		boost::mutex::scoped_lock lock(m_threadQueueMutex);
		
		THREADCALLQUEUE::iterator threadQueueIter = m_threadQueue.find(dwThreadId);
		if(threadQueueIter == m_threadQueue.end())
		{
			// No queue for that thread id, so bail.
			return;
		}

		CALLQUEUE::iterator callQueueIter = std::find((*threadQueueIter).second.begin(), (*threadQueueIter).second.end(), pCallHandler);
		if(callQueueIter == (*threadQueueIter).second.end())
		{
			// The callback was not found in the thread's queue
			return;
		}
		
        // Delete the thread's queue
		(*threadQueueIter).second.erase(callQueueIter);

        // If the threads queue is now empty; delete it
        if((*threadQueueIter).second.empty())
        {
            m_threadQueue.erase(threadQueueIter);
        }
	}

	template<class PickupPolicy>
    CallHandler* CallScheduler<PickupPolicy>::getNextCallFromQueue(DWORD dwThreadId, boost::scoped_ptr<boost::try_mutex::scoped_try_lock>& pCallHandlerLock)
	{
		// Acquire a lock on the thread queue
		boost::mutex::scoped_lock lock(m_threadQueueMutex);
		THREADCALLQUEUE::iterator threadQueueIter;

		if((threadQueueIter = m_threadQueue.find(dwThreadId)) != m_threadQueue.end())
		{
			// Find the queue for the current requested thread
			CALLQUEUE* pCallbackQueue = &((*threadQueueIter).second);

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
					m_threadQueue.erase(threadQueueIter);
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
        boost::scoped_ptr<boost::try_mutex::scoped_try_lock> pCallHandlerLock;

		while((pCallHandler = pSchedulerInstance->getNextCallFromQueue(dwThreadId, pCallHandlerLock)) != NULL)
		{
			// A call handler has been checked out of the structure
			pCallHandler->executeCallback();

			// Once this lock is reset, pCallHandler isn't guaranteed to be valid anymore
			pCallHandlerLock.reset();
		}
	}

    template<class PickupPolicy>
    void CallScheduler<PickupPolicy>::processSynchronousCallHandler(DWORD dwThreadId, CallHandler* pCallHandler, DWORD dwTimeout, boost::scoped_ptr<boost::try_mutex::scoped_lock>& pCallHandlerLock)
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
        pCallHandlerLock.reset(new boost::try_mutex::scoped_lock(*pCallHandler->getAccessMutex()));
    }

    template<class PickupPolicy>
    void CallScheduler<PickupPolicy>::preProcessAsynchronousCallHandler(DWORD dwThreadId, CallHandler* pCallHandler)
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
    }
}