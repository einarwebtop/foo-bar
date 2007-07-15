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
    namespace details
    {
        /*!
        ** @brief A function which throws an exception object with an attached on-cleanup functor.
        ** @param[in] orig the exception object to be thrown.
        ** @param[in] onDeathFunctor a functor, with return type void and no parameters, which will be
        **   executed when the exception object is destroyed.
        ** @remark
        **   The function will create a dummy class which wraps the exception object to be thrown.
        **   When the exception object is free'd, which is usually when the catch block is completed, 
        **   the functor will be executed.
        */
        template<class T>
        void throwHooked(const T& orig, boost::function<void()>& onDeathFunctor)
        {
            #pragma warning(push)
            #pragma warning(disable: 4512)
            class ExHook : public T
            {
            public:
                ExHook(const T& other, const boost::function<void()>& onDeathFunctor) 
                    : T(other), 
                      m_onDeathFunctor(onDeathFunctor) 
                {}

                ExHook(const ExHook& other) 
                    : T(other),
                      m_onDeathFunctor(other.m_onDeathFunctor) 
                {}

                ~ExHook() 
                { m_onDeathFunctor(); }

            private:
                const boost::function<void()> m_onDeathFunctor;
            };
            #pragma warning(pop)

            throw ExHook(orig, onDeathFunctor);
        }

    }
}