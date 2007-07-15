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
    /*!@class FunctorRetvalBinder 
    ** @brief A helper functor and return value class.
    ** A helper class which stores the call functor, and attempts to
    ** grab the return value after a successful call. CallHandler
    ** cannot be templated without major pain in the rest of the code,
    ** so this templated type does the work.
    */
    template<typename T>
    class FunctorRetvalBinder
    {
    public:
        /************************************************************************
        ** Functions
        */

        FunctorRetvalBinder(boost::function<T()> functor, BYTE* pReturnValueBuffer)
            : m_functor(functor),
              m_pReturnValueBuffer(pReturnValueBuffer)
        {}

        FunctorRetvalBinder(boost::function<void()> functor)
            : m_functor(functor),
              m_pReturnValueBuffer(NULL)
        {}

        ~FunctorRetvalBinder();

        /*! 
        ** @brief executes the callback.
        ** @remark 
        **   This function has one variation for T which are non-void,
        **   in which case the return value is saved at the address passed to
        **   the constructor. For T which is void, the return value is
        **   obviously not stored -- or rather, there is no return value.
        */
        inline void execute();

        /*! 
        ** @brief releases any resources currently held by an FunctorRetvalBinder
        **   instance, and deletes the instance itself.
        */
        inline void free()
        { delete this; }

    private:
        /************************************************************************
        ** Variables
        */

        boost::function<T(void)> m_functor;
        BYTE* m_pReturnValueBuffer;
    };

    /************************************************************************
    ** Implementation of non-inline and specialized template member functions
    */

    template<typename T>
    FunctorRetvalBinder<T>::~FunctorRetvalBinder()
    {
        // Destroy the bound return value
        reinterpret_cast<T*>(m_pReturnValueBuffer)->~T();
    }

    template<>
    FunctorRetvalBinder<void>::~FunctorRetvalBinder()
    {
    }

    template<typename T>
    void FunctorRetvalBinder<T>::execute()
    {
        // Copy-construct the object into the allocated buffer, from the return 
        // value of the functor. 
        // The copy constructor of T is as such a requirement for the mechanism to work.
        new (m_pReturnValueBuffer) T(m_functor());
    }

    template<>
    void FunctorRetvalBinder<void>::execute()
    {
        m_functor();
    }
}