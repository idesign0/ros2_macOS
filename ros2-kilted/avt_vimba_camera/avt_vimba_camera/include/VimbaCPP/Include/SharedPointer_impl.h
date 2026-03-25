/*=============================================================================
  Copyright (C) 2012 - 2017 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        SharedPointer_impl.h

  Description: Implementation of an example shared pointer class for the Vimba 
               CPP API.
               (This include file contains example code only.)

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, 
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#ifndef AVT_VMBAPI_SHAREDPOINTER_IMPL_H
#define AVT_VMBAPI_SHAREDPOINTER_IMPL_H

#include <VimbaCPP/Include/SharedPointer.h>
#include <stdexcept>
namespace AVT {
namespace VmbAPI {

    template <class T>
    ref_count<T>::ref_count(const ref_count &rRefCount)
    {
    }

    template <class T>
    ref_count<T>& ref_count<T>::operator = (const ref_count &rRefCount)
    {
        //This is a dummy method to satisfy the compiler
        return *this;
    }

    template <class T>
    ref_count<T>::ref_count(T *pObject)
        :   m_pObject(pObject)
        ,   m_nCount(1)
    {
    }

    template <class T>
    ref_count<T>::~ref_count()
    {
        if(NULL != m_pObject)
        {
            delete m_pObject;
        }
        
        m_Mutex.Unlock();
    }

    template <class T>
    void ref_count<T>::inc()
    {
        m_Mutex.Lock();
        
        m_nCount++;

        m_Mutex.Unlock();
    }

    template <class T>
    void ref_count<T>::dec()
    {
        m_Mutex.Lock();
        if( m_nCount == 0 )
        {
            throw std::logic_error("shared pointer, used incorectly");
        }
        if(m_nCount > 1)
        {
            m_nCount--;

            m_Mutex.Unlock();
        }
        else
        {
            // m_Mutex will be unlocked in d'tor
            delete this;
        }
    }

    template <class T>
    long ref_count<T>::use_count() const
    {
        return m_nCount;
    }

    template <class T>
    template <class T2>
    void shared_ptr<T>::swap(T2 &rValue1, T2 &rValue2)
    {
        T2 buffer = rValue1;
        rValue1 = rValue2;
        rValue2 = buffer;
    }

    template <class T>
    shared_ptr<T>::shared_ptr()
        :   m_pRefCount(NULL)
        ,   m_pObject(NULL)
    {
    }
    
    template <class T>
    template <class T2>
    shared_ptr<T>::shared_ptr(T2 *pObject)
        :   m_pRefCount(NULL)
        ,   m_pObject(NULL)
    {
        m_pRefCount = new ref_count<T2>(pObject);
        if(NULL == m_pRefCount)
        {
            delete pObject;

            throw std::bad_alloc();
        }

        m_pObject = pObject;
    }
    
    template <class T>
    template <class T2>
    shared_ptr<T>::shared_ptr(const shared_ptr<T2> &rSharedPointer)
        :   m_pRefCount(NULL)
        ,   m_pObject(NULL)
    {
        if(NULL != rSharedPointer.m_pRefCount)
        {
            rSharedPointer.m_pRefCount->inc();

            m_pRefCount = rSharedPointer.m_pRefCount;
            m_pObject = rSharedPointer.m_pObject;
        }
    }

    template <class T>
    template <class T2>
    shared_ptr<T>::shared_ptr(const shared_ptr<T2> &rSharedPointer, dynamic_cast_tag)
        :   m_pRefCount(NULL)
        ,   m_pObject(NULL)
    {
        if(NULL != rSharedPointer.m_pRefCount)
        {
            T *pObject = dynamic_cast<T*>(rSharedPointer.m_pObject);
            if(NULL != pObject)
            {
                rSharedPointer.m_pRefCount->inc();

                m_pRefCount = rSharedPointer.m_pRefCount;
                m_pObject = pObject;
            }
        }
    }

    template <class T>
    shared_ptr<T>::shared_ptr(const shared_ptr &rSharedPointer)
        :   m_pRefCount(NULL)
        ,   m_pObject(NULL)
    {
        if(NULL != rSharedPointer.m_pRefCount)
        {
            rSharedPointer.m_pRefCount->inc();

            m_pRefCount = rSharedPointer.m_pRefCount;
            m_pObject = rSharedPointer.m_pObject;
        }
    }

    template <class T>
    shared_ptr<T>::~shared_ptr()
    {
        if(NULL != m_pRefCount)
        {
            m_pRefCount->dec();
            m_pRefCount = NULL;
            m_pObject = NULL;
        }
    }

    template <class T>
    template <class T2>
    shared_ptr<T>& shared_ptr<T>::operator = (const shared_ptr<T2> &rSharedPointer)
    {
        shared_ptr(rSharedPointer).swap(*this);

        return *this;
    }

    template <class T>
    shared_ptr<T>& shared_ptr<T>::operator = (const shared_ptr &rSharedPointer)
    {
        shared_ptr(rSharedPointer).swap(*this);

        return *this;
    }

    template <class T>
    void shared_ptr<T>::reset()
    {
        shared_ptr().swap(*this);
    }
    
    template <class T>
    template <class T2>
    void shared_ptr<T>::reset(T2 *pObject)
    {
        shared_ptr(pObject).swap(*this);
    }

    template <class T>
    T* shared_ptr<T>::get() const
    {
        return m_pObject;
    }
    
    template <class T>
    T& shared_ptr<T>::operator * () const
    {
        return *m_pObject;
    }
    
    template <class T>
    T* shared_ptr<T>::operator -> () const
    {
        return m_pObject;
    }
    
    template <class T>
    long shared_ptr<T>::use_count() const
    {
        if(NULL == m_pRefCount)
        {
            return 0;
        }

        return m_pRefCount->use_count();
    }
    
    template <class T>
    bool shared_ptr<T>::unique() const
    {
        return (use_count() == 1);
    }

    template <class T>
    void shared_ptr<T>::swap(shared_ptr &rSharedPointer)
    {
        swap(m_pObject, rSharedPointer.m_pObject);
        swap(m_pRefCount, rSharedPointer.m_pRefCount);
    }

    template<class T, class T2>
    shared_ptr<T> dynamic_pointer_cast(const shared_ptr<T2> &rSharedPointer)
    {
        return shared_ptr<T>(rSharedPointer, dynamic_cast_tag());
    }

    template <class T1, class T2>
    bool operator==(const shared_ptr<T1>& sp1, const shared_ptr<T2>& sp2)
    {
        return sp1.get() == sp2.get();
    }

    template <class T1, class T2>
    bool operator!=(const shared_ptr<T1>& sp1, const shared_ptr<T2>& sp2)
    {
        return sp1.get() != sp2.get();
    }

}} //namespace AVT::VmbAPI

#endif //AVT_VMBAPI_SHAREDPOINTER_IMPL_H
