/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FeatureContainer.h

  Description: Definition of class AVT::VmbAPI::FeatureContainer.

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

#ifndef AVT_VMBAPI_FEATURECONTAINER_H
#define AVT_VMBAPI_FEATURECONTAINER_H

#include <VimbaC/Include/VmbCommonTypes.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/BasicLockable.h>
#include <VimbaCPP/Include/SharedPointerDefines.h>
#include <VimbaCPP/Include/Feature.h>

namespace AVT {
namespace VmbAPI {

class FeatureContainer : public virtual BasicLockable
{
  public:

    //
    // Method:      FeatureContainer constructor
    //
    // Purpose:     Creates an instance of class FeatureContainer
    //
    IMEXPORT FeatureContainer();

    //
    // Method:      FeatureContainer destructor
    //
    // Purpose:     Destroys an instance of class FeatureContainer
    //
    IMEXPORT ~FeatureContainer();

    //
    // Method:      GetFeatureByName()
    //
    // Purpose:     Gets one particular feature of a feature container (e.g. a camera)
    //
    // Parameters:
    //
    // [in ]    const char*         name                The name of the feature to get
    // [out]    FeaturePtr&         pFeature            The queried feature
    //
    // Returns:
    //
    //  - VmbErrorSuccess:          If no error
    //  - VmbErrorDeviceNotOpen:    Base feature class (e.g. Camera) was not opened.
    //  - VmbErrorBadParameter:     "name" is NULL.
    //
    IMEXPORT VmbErrorType GetFeatureByName( const char *pName, FeaturePtr &pFeature );
    
    //
    // Method:      GetFeatures()
    //
    // Purpose:     Gets all features of a feature container (e.g. a camera)
    //
    // Parameters:
    //
    // [out]    FeaturePtrVector&   features        The container for all queried features
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorBadParameter:  "features" is empty.
    //
    // Details:   Once queried, this information remains static throughout the object's lifetime
    //
    VmbErrorType GetFeatures( FeaturePtrVector &features );

    VmbHandle_t GetHandle() const;

  protected:
    // Sets the C handle of a feature container
    void SetHandle( const VmbHandle_t handle );
    
    // Sets the C handle of a feature container to NULL
    void RevokeHandle();
    
    // Sets the back reference to feature container that each feature holds to NULL
    // and resets all known features
    void Reset();

  private:
    struct Impl;
    Impl *m_pImpl;

    IMEXPORT VmbErrorType GetFeatures( FeaturePtr *pFeatures, VmbUint32_t &size );

    // No copy ctor
    FeatureContainer( const FeatureContainer& );
    // No assignment operator
    FeatureContainer& operator=( const FeatureContainer& );
};

#include <VimbaCPP/Include/FeatureContainer.hpp>

}} // namespace AVT::VmbAPI

#endif
