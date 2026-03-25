/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        SharedPointer.h

  Description: Definition of an example shared pointer class that can be 
               used with the Vimba CPP API.
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

#ifndef AVT_VMBAPI_SHAREDPOINTER_H
#define AVT_VMBAPI_SHAREDPOINTER_H

#include <VimbaCPP/Include/Mutex.h>

namespace AVT {
namespace VmbAPI {

    //Coding style of this file is different to mimic the shared_ptr classes of std and boost.

    struct dynamic_cast_tag
    {
    };

    class ref_count_base
    {
    public:
        virtual ~ref_count_base() {;}

        virtual void inc() = 0;
        virtual void dec() = 0;
        virtual long use_count() const = 0;
    };

    template <class T>
    class ref_count : public virtual AVT::VmbAPI::ref_count_base
    {
    private:
        T               *m_pObject;
        long            m_nCount;
        Mutex           m_Mutex;

        ref_count(const ref_count &rRefCount);
        ref_count& operator = (const ref_count &rRefCount);

    public:
        explicit ref_count(T *pObject);
        virtual ~ref_count();

        virtual void inc();
        virtual void dec();
        virtual long use_count() const;
    };

    template <class T>
    class shared_ptr
    {
    private:
        typedef shared_ptr<T> this_type;
        
        template<class T2>
        friend class shared_ptr;

        AVT::VmbAPI::ref_count_base *m_pRefCount;
        T                           *m_pObject;

        template <class T2>
        static void swap(T2 &rValue1, T2 &rValue2);

    public:
        shared_ptr();
        template <class T2>
        explicit shared_ptr(T2 *pObject);
        shared_ptr(const shared_ptr &rSharedPointer);
        template <class T2>
        shared_ptr(const shared_ptr<T2> &rSharedPointer);
        template <class T2>
        shared_ptr(const shared_ptr<T2> &rSharedPointer, dynamic_cast_tag);

        virtual ~shared_ptr();

        shared_ptr& operator = (const shared_ptr &rSharedPointer);
        template <class T2>
        shared_ptr& operator = (const shared_ptr<T2> &rSharedPointer);
        
        void reset();
        template <class T2>
        void reset(T2 *pObject);

        T* get() const;
        T& operator * () const;
        T* operator -> () const;
        long use_count() const;
        bool unique() const;
        
        typedef T* this_type::*unspecified_bool_type;

        operator unspecified_bool_type () const
        {
            if(m_pObject == 0)
            {
                return 0;
            }
            
            return &this_type::m_pObject;
        }

        void swap(shared_ptr &rSharedPointer);
    };

    template<class T, class T2>
    shared_ptr<T> dynamic_pointer_cast(const shared_ptr<T2> &rSharedPointer);

    template<class T1, class T2>
    bool operator==(const shared_ptr<T1>& sp1, const shared_ptr<T2>& sp2);
    template<class T1, class T2>
    bool operator!=(const shared_ptr<T1>& sp1, const shared_ptr<T2>& sp2);

}} //namespace AVT::VmbAPI

#include <VimbaCPP/Include/SharedPointer_impl.h>

#endif //AVT_VMBAPI_SHAREDPOINTER_H
