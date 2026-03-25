/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        UserSharedPointerDefines.h

  Description: Definition of macros for using different shared pointer 
               implementations.

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

#ifndef AVT_VMBAPI_USERSHAREDPOINTERDEFINES_H
#define AVT_VMBAPI_USERSHAREDPOINTERDEFINES_H

// 
// Vimba C++ API does not necessarily rely on AVT::VmbAPI::shared_ptr. You might want to use your own shared pointer type or the one that ships with your
// implementation of the C++ standard.
// To use a custom shared pointer implementation simply add the define USER_SHARED_POINTER to your project / compiler settings and complete this header file.
//


// Add all your required shared pointer implementation headers here.
// HINT: #include <memory> is used for std::shared_ptr


namespace AVT {
namespace VmbAPI {

// Set the calls for your implementation of the shared pointer functions
// a) Declaration
// b) Reset with argument
// c) Reset without argument
// d) == operator
// e) NULL test
// f) Access to underlying raw pointer
// g) Dynamic cast of shared pointer

// a) This is the define for a declaration.
#define SP_DECL( T )            std::shared_ptr<T>
// b) This is the define for setting an existing shared pointer.
#define SP_SET( sp, rawPtr )    (sp).reset( rawPtr )
// c) This is the define for resetting without an argument to decrease the ref count.
#define SP_RESET( sp )          (sp).reset()
// d) This is the define for the equal operator. Shared pointers are usually considered equal when the raw pointers point to the same address.
#define SP_ISEQUAL( sp1, sp2 )  ( (sp1) == (sp2) )
// e) This is the define for the NULL check.
#define SP_ISNULL( sp )         ( NULL == (sp) )
// f) This is the define for the raw pointer access. This is usually accomplished through the dereferencing operator (->).
#define SP_ACCESS( sp )         (sp).get()
// g) This is the define for the dynamic cast of the pointer.
#define SP_DYN_CAST( sp, T )    std::dynamic_pointer_cast<T>(sp)

// These are all uses of a SP_DECL shared_ptr declaration
class Interface;
typedef SP_DECL( Interface ) InterfacePtr;

class Camera;
typedef SP_DECL( Camera ) CameraPtr;

class Feature;
typedef SP_DECL( Feature ) FeaturePtr;

class FeatureContainer;
typedef SP_DECL( FeatureContainer ) FeatureContainerPtr;

class IFeatureObserver;
typedef SP_DECL( IFeatureObserver ) IFeatureObserverPtr;

class Frame;
typedef SP_DECL( Frame ) FramePtr;

class FrameHandler;
typedef SP_DECL( FrameHandler ) FrameHandlerPtr;

class IFrameObserver;
typedef SP_DECL( IFrameObserver ) IFrameObserverPtr;

class AncillaryData;
typedef SP_DECL( AncillaryData ) AncillaryDataPtr;

class ConstAncillaryData;
typedef SP_DECL( const AncillaryData ) ConstAncillaryDataPtr;

class ICameraFactory;
typedef SP_DECL( ICameraFactory ) ICameraFactoryPtr;

class ICameraListObserver;
typedef SP_DECL( ICameraListObserver ) ICameraListObserverPtr;

class IInterfaceListObserver;
typedef SP_DECL( IInterfaceListObserver ) IInterfaceListObserverPtr;

class Mutex;
typedef SP_DECL( Mutex ) MutexPtr;

class BasicLockable;
typedef SP_DECL( BasicLockable ) BasicLockablePtr;

}} // Namespace AVT::VmbAPI
   

#endif //AVT_VMBAPI_USERSHAREDPOINTERDEFINES_H
