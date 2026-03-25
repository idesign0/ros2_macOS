/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BaseFeature.h

  Description: Definition of base class AVT::VmbAPI::BaseFeature.

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

#ifndef AVT_VMBAPI_BASEFEATURE_H
#define AVT_VMBAPI_BASEFEATURE_H

#include <VimbaC/Include/VimbaC.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/BasicLockable.h>
#include <VimbaCPP/Include/Feature.h>

namespace AVT {
namespace VmbAPI {

class BaseFeature : public virtual BasicLockable
{
  friend class Feature;

  public:
    BaseFeature( const VmbFeatureInfo_t *pFeatureInfo, FeatureContainer *pFeatureContainer );
    virtual ~BaseFeature();

    IMEXPORT virtual    VmbErrorType GetValue( VmbInt64_t &value ) const;
    IMEXPORT virtual    VmbErrorType GetValue( double &value ) const;
    IMEXPORT virtual    VmbErrorType GetValue( bool &value ) const;

    IMEXPORT virtual    VmbErrorType SetValue( const VmbInt32_t &value );
    IMEXPORT virtual    VmbErrorType SetValue( const VmbInt64_t &value );
    IMEXPORT virtual    VmbErrorType SetValue( const double &value );
    IMEXPORT virtual    VmbErrorType SetValue( const char *pValue );
    IMEXPORT virtual    VmbErrorType SetValue( bool value );

    IMEXPORT virtual    VmbErrorType GetEntry( EnumEntry &entry, const char * pStrEntryName ) const;

    IMEXPORT virtual    VmbErrorType GetRange( VmbInt64_t &minimum, VmbInt64_t &maximum ) const;
    IMEXPORT virtual    VmbErrorType GetRange( double &minimum, double &maximum ) const;

    IMEXPORT virtual    VmbErrorType HasIncrement( VmbBool_t &incrementSupported) const;
    IMEXPORT virtual    VmbErrorType GetIncrement( VmbInt64_t &increment ) const;
    IMEXPORT virtual    VmbErrorType GetIncrement( double &increment ) const;

    IMEXPORT virtual    VmbErrorType IsValueAvailable( const char *pValue, bool &available ) const;
    IMEXPORT virtual    VmbErrorType IsValueAvailable( const VmbInt64_t value, bool &available ) const;

    IMEXPORT virtual    VmbErrorType RunCommand();
    IMEXPORT virtual    VmbErrorType IsCommandDone( bool &isDone ) const;

    IMEXPORT            VmbErrorType GetDataType( VmbFeatureDataType &dataType ) const;
    IMEXPORT            VmbErrorType GetFlags( VmbFeatureFlagsType &flags ) const;
    IMEXPORT            VmbErrorType GetPollingTime( VmbUint32_t &pollingTime ) const;
    IMEXPORT            VmbErrorType GetVisibility( VmbFeatureVisibilityType &visibility ) const;
    IMEXPORT            VmbErrorType IsReadable( bool &isReadable );
    IMEXPORT            VmbErrorType IsWritable( bool &isWritable );
    IMEXPORT            VmbErrorType IsStreamable( bool &isStreamable ) const;

    IMEXPORT            VmbErrorType RegisterObserver( const IFeatureObserverPtr &observer );
    IMEXPORT            VmbErrorType UnregisterObserver( const IFeatureObserverPtr &observer );

    void ResetFeatureContainer();

  protected:
    // Copy of feature infos
    struct FeatureInfo
    {
        std::string                     name;                   // Verbose name
        VmbFeatureData_t                featureDataType;        // Data type of this feature
        VmbFeatureFlags_t               featureFlags;           // Access flags for this feature
        bool                            hasAffectedFeatures;    // true if the feature selects or invalidates other features
        bool                            hasSelectedFeatures;    // true if the feature selects other features
        std::string                     category;               // Category this feature can be found in
        std::string                     displayName;            // Feature name to be used in GUIs
        VmbUint32_t                     pollingTime;            // Predefined polling time for volatile features
        std::string                     unit;                   // Measuring unit as given in the XML file
        std::string                     representation;         // Representation of a numeric feature
        VmbFeatureVisibility_t          visibility;             // GUI visibility
        std::string                     tooltip;                // Short description
        std::string                     description;            // Longer description
        std::string                     sfncNamespace;          // Namespace this feature resides in
        bool                            isStreamable;           // Feature can be stored or loaded from/into a file
    };

    FeatureInfo m_featureInfo;

    FeatureContainer *m_pFeatureContainer;

  private:
    // Default ctor
    BaseFeature();

    // Copy ctor
    BaseFeature( const BaseFeature& );

    struct Impl;
    Impl *m_pImpl;

    // Array functions to pass data across DLL boundaries
    IMEXPORT virtual    VmbErrorType GetValue( char * const pValue, VmbUint32_t &length ) const;
    IMEXPORT virtual    VmbErrorType GetValue( VmbUchar_t *pValue, VmbUint32_t &size, VmbUint32_t &sizeFilled ) const;
    IMEXPORT virtual    VmbErrorType GetValues( const char **pValues, VmbUint32_t &size );
    IMEXPORT virtual    VmbErrorType GetValues( VmbInt64_t *pValues, VmbUint32_t &Size );

    IMEXPORT virtual    VmbErrorType SetValue( const VmbUchar_t *pValue, VmbUint32_t size );

    IMEXPORT virtual    VmbErrorType GetEntries( EnumEntry *pEntries, VmbUint32_t &size );

    IMEXPORT virtual    VmbErrorType GetName( char * const pName, VmbUint32_t &length ) const;
    IMEXPORT            VmbErrorType GetDisplayName( char * const pDisplayName, VmbUint32_t &length ) const;
    IMEXPORT            VmbErrorType GetCategory( char * const pCategory, VmbUint32_t &length ) const;
    IMEXPORT            VmbErrorType GetUnit( char * const pUnit, VmbUint32_t &length ) const;
    IMEXPORT            VmbErrorType GetRepresentation( char * const pRepresentation, VmbUint32_t &length ) const;
    IMEXPORT            VmbErrorType GetToolTip( char * const pToolTip, VmbUint32_t &length ) const;
    IMEXPORT            VmbErrorType GetDescription( char * const pDescription, VmbUint32_t &length ) const;
    IMEXPORT            VmbErrorType GetSFNCNamespace( char * const pSFNCNamespace, VmbUint32_t &length ) const;
    IMEXPORT            VmbErrorType GetAffectedFeatures( FeaturePtr *pAffectedFeatures, VmbUint32_t &nSize );
    IMEXPORT            VmbErrorType GetSelectedFeatures( FeaturePtr *pSelectedFeatures, VmbUint32_t &nSize );
};

}} // namespace AVT::VmbAPI

#endif
