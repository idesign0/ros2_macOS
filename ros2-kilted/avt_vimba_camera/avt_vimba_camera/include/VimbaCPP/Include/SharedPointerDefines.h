/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        SharedPointerDefines.h

  Description: Definition of macros for using the standard shared pointer 
               (std::tr1) for Vimba CPP API.

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

#ifndef AVT_VMBAPI_SHAREDPOINTERDEFINES_H
#define AVT_VMBAPI_SHAREDPOINTERDEFINES_H

//
// If your version of STL does not provide a shared pointer implementation please see UserSharedPointerDefines.h for information on 
// how to use another shared pointer than std::shared_ptr.
//

#ifndef USER_SHARED_POINTER

    #include <VimbaCPP/Include/SharedPointer.h>

    namespace AVT {
    namespace VmbAPI {

    #define SP_DECL( T )            AVT::VmbAPI::shared_ptr<T>
    #define SP_SET( sp, rawPtr )    (sp).reset( (rawPtr) )
    #define SP_RESET( sp )          (sp).reset()
    #define SP_ISEQUAL( sp1, sp2 )  ( (sp1) == (sp2) )
    #define SP_ISNULL( sp )         ( NULL == (sp) )
    #define SP_ACCESS( sp )         (sp).get()
    #define SP_DYN_CAST( sp, T )    AVT::VmbAPI::dynamic_pointer_cast<T>(sp)

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
    typedef SP_DECL( AncillaryData )            AncillaryDataPtr;
    typedef SP_DECL( const AncillaryData )      ConstAncillaryDataPtr;

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

    }}

#else
    #include <VimbaCPP/Include/UserSharedPointerDefines.h>
#endif


#endif // AVT_VMBAPI_SHAREDPOINTERDEFINES_H
