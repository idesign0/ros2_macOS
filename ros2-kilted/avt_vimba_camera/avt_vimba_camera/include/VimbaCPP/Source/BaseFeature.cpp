/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BaseFeature.cpp

  Description: Implementation of base class AVT::VmbAPI::BaseFeature.

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

#pragma warning(disable:4996)
#include <VimbaCPP/Source/BaseFeature.h>
#pragma warning(default:4996)

#include <VimbaCPP/Include/FeatureContainer.h>
#include <VimbaCPP/Include/VimbaSystem.h>
#include <VimbaCPP/Source/ConditionHelper.h>
#include <VimbaCPP/Source/Helper.h>

namespace AVT {
namespace VmbAPI {

struct BaseFeature::Impl
{
    LockableVector<IFeatureObserverPtr> m_observers;

    FeaturePtrVector m_affectedFeatures;
    FeaturePtrVector m_selectedFeatures;
    bool m_bAffectedFeaturesFetched;
    bool m_bSelectedFeaturesFetched;

    ConditionHelper m_observersConditionHelper;
    ConditionHelper m_conditionHelper;

    static void VMB_CALL InvalidationCallback( const VmbHandle_t handle, const char *name, void *context );
};

BaseFeature::BaseFeature( const VmbFeatureInfo_t *pFeatureInfo, FeatureContainer *pFeatureContainer )
    :   m_pImpl( new Impl() )
    ,   m_pFeatureContainer( pFeatureContainer )
{
    m_pImpl->m_bAffectedFeaturesFetched = false;
    m_pImpl->m_bSelectedFeaturesFetched = false;

    if ( NULL != pFeatureInfo )
    {
        m_featureInfo.category.assign( pFeatureInfo->category ? pFeatureInfo->category : "" );
        m_featureInfo.description.assign( pFeatureInfo->description ? pFeatureInfo->description : "" );
        m_featureInfo.displayName.assign( pFeatureInfo->displayName ? pFeatureInfo->displayName : "" );
        m_featureInfo.featureDataType = pFeatureInfo->featureDataType;
        m_featureInfo.featureFlags = pFeatureInfo->featureFlags;
        m_featureInfo.hasAffectedFeatures = pFeatureInfo->hasAffectedFeatures;
        m_featureInfo.hasSelectedFeatures = pFeatureInfo->hasSelectedFeatures;
        m_featureInfo.name.assign( pFeatureInfo->name ? pFeatureInfo->name : "" );
        m_featureInfo.pollingTime = pFeatureInfo->pollingTime;
        m_featureInfo.representation.assign( pFeatureInfo->representation ? pFeatureInfo->representation : "" );
        m_featureInfo.sfncNamespace.assign( pFeatureInfo->sfncNamespace ? pFeatureInfo->sfncNamespace : "" );
        m_featureInfo.tooltip.assign( pFeatureInfo->tooltip ? pFeatureInfo->tooltip : "" );
        m_featureInfo.unit.assign( pFeatureInfo->unit ? pFeatureInfo->unit : "" );
        m_featureInfo.visibility = pFeatureInfo->visibility;
        m_featureInfo.isStreamable = pFeatureInfo->isStreamable;

        if ( NULL == m_pFeatureContainer ) // m_pFeatureContainer == NULL (Just for safety)
        {
            // Do some logging
            LOG_FREE_TEXT( "No valid feature container pointer passed" );
        }
    }
    else // m_featureInfo == NULL (Just for safety)
    {
        // Do some logging
        LOG_FREE_TEXT( "No valid feature info pointer passed" );
    }
}

BaseFeature::BaseFeature()
{
    // No default ctor
}

BaseFeature::BaseFeature( const BaseFeature& )
{
    // No copy ctor
}

BaseFeature::~BaseFeature()
{
    // Before destruction we unregister all observers and all callbacks
    ResetFeatureContainer();

    delete m_pImpl;
}

// Unregisters all observers before it resets the feature container pointer.
void BaseFeature::ResetFeatureContainer()
{
    if ( NULL != m_pFeatureContainer )
    {
        // Camera still open
        if ( NULL != m_pFeatureContainer->GetHandle() )
        {
            VmbFeatureInvalidationUnregister( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), m_pImpl->InvalidationCallback );
        }

        // Begin exclusive write lock this feature
        if ( true == m_pImpl->m_conditionHelper.EnterWriteLock( GetMutex(), true ))
        {
            m_pFeatureContainer = NULL;

            // End write lock this feature
            m_pImpl->m_conditionHelper.ExitWriteLock( GetMutex() );
        }
        else
        {
            LOG_FREE_TEXT( "Could not reset a feature's feature container reference. ");
        }
        
    }
    
    // Begin exclusive write lock observer list
    if ( true == m_pImpl->m_observersConditionHelper.EnterWriteLock( m_pImpl->m_observers, true ))
    {
        m_pImpl->m_observers.Vector.clear();
        
        // End write lock observer list
        m_pImpl->m_observersConditionHelper.ExitWriteLock( m_pImpl->m_observers );
    }
}

void VMB_CALL BaseFeature::Impl::InvalidationCallback( const VmbHandle_t handle, const char * /*name*/, void *context )
{
    BaseFeature *pFeature = (BaseFeature*)context;
    if ( NULL != pFeature )
    {
        if ( NULL != handle )
        {
            // Begin read lock this feature
            if ( true == pFeature->m_pImpl->m_conditionHelper.EnterReadLock( pFeature->GetMutex() ))
            {
                if ( NULL != pFeature->m_pFeatureContainer )
                {
                    FeaturePtr pFeaturePtrFromMap;
                    if ( VmbErrorSuccess == pFeature->m_pFeatureContainer->GetFeatureByName( pFeature->m_featureInfo.name.c_str(), pFeaturePtrFromMap ) )
                    {
                        // Begin read lock observer list
                        if ( true == pFeature->m_pImpl->m_observersConditionHelper.EnterReadLock( pFeature->m_pImpl->m_observers ))
                        {
                            for (   IFeatureObserverPtrVector::iterator iter = pFeature->m_pImpl->m_observers.Vector.begin();
                                    pFeature->m_pImpl->m_observers.Vector.end() != iter;
                                    ++iter)
                            {
                                SP_ACCESS(( *iter ))->FeatureChanged( pFeaturePtrFromMap );
                            }

                            // End read lock observer list
                            pFeature->m_pImpl->m_observersConditionHelper.ExitReadLock( pFeature->m_pImpl->m_observers );
                        }
                        else
                        {
                            LOG_FREE_TEXT( "Could not lock feature observer list.")
                        }
                    }
                    else // GetFeatureByName() failed
                    {
                        // Do some logging
                        LOG_FREE_TEXT( "GetFeatureByName failed" )
                    }
                }
                else // m_pFeatureContainer == NULL (Feature destroyed or device closed / destroyed)
                {
                    // Do some logging
                    LOG_FREE_TEXT( "Feature destroyed or device closed / destroyed" );
                }

                // End read lock this feature
                pFeature->m_pImpl->m_conditionHelper.ExitReadLock( pFeature->GetMutex() );
            }
            else
            {
                LOG_FREE_TEXT( "Could not lock feature.")
            }
        }
        else // m_handle == NULL (device closed / destroyed)
        {
            // Do some logging
            LOG_FREE_TEXT( "Device closed / destroyed" )
        }
    }
    else // pFeature == NULL (Just for safety)
    {
        // Do some logging
        LOG_FREE_TEXT( "Feature pointer is null" )
    }
}

VmbErrorType BaseFeature::RegisterObserver( const IFeatureObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }    

    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbError_t res = VmbErrorSuccess;

    // Begin write lock observer list
    if ( true == m_pImpl->m_observersConditionHelper.EnterWriteLock( m_pImpl->m_observers ))
    {
        // The very same observer cannot be registered twice
        for ( size_t i=0; i<m_pImpl->m_observers.Vector.size(); ++i )
        {
            if ( SP_ISEQUAL( rObserver, m_pImpl->m_observers.Vector[i] ))
            {
                res = VmbErrorInvalidCall;
                break;
            }
        }

        if ( VmbErrorSuccess == res )
        {
            if ( 0 == m_pImpl->m_observers.Vector.size() )
            {
                res = VmbFeatureInvalidationRegister( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), m_pImpl->InvalidationCallback, this );
            }

            if ( VmbErrorSuccess == res )
            {
                m_pImpl->m_observers.Vector.push_back( rObserver );
            }
        }
        
        // End write lock observer list
        m_pImpl->m_observersConditionHelper.ExitWriteLock( m_pImpl->m_observers );
    }

    return (VmbErrorType)res;
}

VmbErrorType BaseFeature::UnregisterObserver( const IFeatureObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }    

    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbError_t res = VmbErrorNotFound;

    // Begin exclusive write lock observer list
    if ( true == m_pImpl->m_observersConditionHelper.EnterWriteLock( m_pImpl->m_observers, true ))
    {
        for (   IFeatureObserverPtrVector::iterator iter = m_pImpl->m_observers.Vector.begin();
                m_pImpl->m_observers.Vector.end() != iter;)
        {
            if ( SP_ISEQUAL( rObserver, *iter ))
            {
                // If we are about to unregister the last observer we cancel all invalidation notifications
                if ( 1 == m_pImpl->m_observers.Vector.size() )
                {
                    res = VmbFeatureInvalidationUnregister( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), m_pImpl->InvalidationCallback );
                }
                if (    VmbErrorSuccess == res
                     || 1 < m_pImpl->m_observers.Vector.size() )
                {
                    iter = m_pImpl->m_observers.Vector.erase( iter );
                    res = VmbErrorSuccess;
                }
                break;
            }
            else
            {
                ++iter;
            }
        }

        // End write lock observer list
        m_pImpl->m_observersConditionHelper.ExitWriteLock( m_pImpl->m_observers );
    }
    else
    {
        LOG_FREE_TEXT( "Could not lock feature observer list.")
        res = VmbErrorInternalFault;
    }
    
    return (VmbErrorType)res;
}

// Gets the value of a feature of type VmbFeatureDataInt
VmbErrorType BaseFeature::GetValue( VmbInt64_t & /*rnValue*/ ) const
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataInt
VmbErrorType BaseFeature::SetValue( const VmbInt64_t & /*rnValue*/ )
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataInt
VmbErrorType BaseFeature::SetValue( const VmbInt32_t & /*rnValue*/ )
{
    return VmbErrorWrongType;
}

// Gets the range of a feature of type VmbFeatureDataInt
VmbErrorType BaseFeature::GetRange( VmbInt64_t & /*rnMinimum*/, VmbInt64_t & /*rnMaximum*/ ) const
{
    return VmbErrorWrongType;
}

VmbErrorType BaseFeature::HasIncrement( VmbBool_t & /*incrementSupported*/) const
{
    return VmbErrorWrongType;
}
// Gets the increment of a feature of type VmbFeatureDataInt
VmbErrorType BaseFeature::GetIncrement( VmbInt64_t & /*rnIncrement*/ ) const
{
    return VmbErrorWrongType;
}

// Gets the increment of a feature of type VmbFeatureDataFloat
VmbErrorType BaseFeature::GetIncrement( double & /*rnIncrement*/ ) const
{
    return VmbErrorWrongType;
}

// Gets the value of a feature of type VmbFeatureDataFloat
VmbErrorType BaseFeature::GetValue( double & /*rfValue*/) const
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataFloat
VmbErrorType BaseFeature::SetValue( const double & /*rfValue*/ )
{
    return VmbErrorWrongType;
}

// Gets the range of a feature of type VmbFeatureDataFloat
VmbErrorType BaseFeature::GetRange( double & /*rfMinimum*/, double & /*rfMaximum*/ ) const
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataEnum
// Sets the value of a feature of type VmbFeatureDataString
VmbErrorType BaseFeature::SetValue( const char * /*pStrValue*/ )
{
    return VmbErrorWrongType;
}

// Gets the enum entry of a feature of type VmbFeatureDataEnum
VmbErrorType BaseFeature::GetEntry( EnumEntry & /*entry*/, const char * /*pStrEntryName*/ ) const
{
    return VmbErrorWrongType;
}

// Gets all possible enum entries of a feature of type VmbFeatureDataEnum
VmbErrorType BaseFeature::GetEntries( EnumEntry * /*pEnumEntries*/, VmbUint32_t & /*size*/ )
{
    return VmbErrorWrongType;
}

// Gets all possible values as string of a feature of type VmbFeatureDataEnum
VmbErrorType BaseFeature::GetValues( const char ** /*pStrValues*/, VmbUint32_t & /*rnSize*/ )
{
    return VmbErrorWrongType;
}

// Gets all possible values as integer of a feature of type VmbFeatureDataEnum
VmbErrorType BaseFeature::GetValues( VmbInt64_t * /*pnValues*/, VmbUint32_t & /*rnSize*/ )
{
    return VmbErrorWrongType;
}

// Indicates whether a particular enum value as string of a feature of type VmbFeatureDataEnum is available
VmbErrorType BaseFeature::IsValueAvailable( const char * /*pStrValue*/, bool & /*bAvailable*/ ) const
{
    return VmbErrorWrongType;
}

// Indicates whether a particular enum value as integer of a feature of type VmbFeatureDataEnum is available
VmbErrorType BaseFeature::IsValueAvailable( const VmbInt64_t /*nValue*/, bool & /*bAvailable*/ ) const
{
    return VmbErrorWrongType;
}

// Gets the value of a feature of type VmbFeatureDataString
// Gets the value of a feature of type VmbFeatureDataEnum
VmbErrorType BaseFeature::GetValue( char * const /*pStrValue*/, VmbUint32_t & /*length*/ ) const
{
    return VmbErrorWrongType;
}

// Gets the value of a feature of type VmbFeatureDataBool
VmbErrorType BaseFeature::GetValue( bool & /*rbValue*/ ) const
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataBool
VmbErrorType BaseFeature::SetValue( bool /*bValue*/ )
{
    return VmbErrorWrongType;
}

// Executes a feature of type VmbFeatureDataCommand
VmbErrorType BaseFeature::RunCommand() 
{
    return VmbErrorWrongType;
}

// Indicates whether a feature of type VmbFeatureDataCommand finished execution
VmbErrorType BaseFeature::IsCommandDone( bool & /*bIsDone*/ ) const
{
    return VmbErrorWrongType;
}

// Gets the value of a feature of type VmbFeatureDataRaw
VmbErrorType BaseFeature::GetValue( VmbUchar_t * /*pValue*/, VmbUint32_t & /*rnSize*/, VmbUint32_t & /*rnSizeFilled*/ ) const
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataRaw
VmbErrorType BaseFeature::SetValue( const VmbUchar_t * /*pValue*/, VmbUint32_t /*nSize*/ )
{
    return VmbErrorWrongType;
}

VmbErrorType BaseFeature::GetName( char * const pStrName, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrName )
    {
        rnLength = (VmbUint32_t)m_featureInfo.name.length();
        res = VmbErrorSuccess;
    }
    else if ( m_featureInfo.name.length() <= rnLength )
    {
        std::copy( m_featureInfo.name.begin(), m_featureInfo.name.end(), pStrName );
        rnLength = (VmbUint32_t)m_featureInfo.name.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType BaseFeature::GetDisplayName( char * const pStrDisplayName, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrDisplayName )
    {
        rnLength = (VmbUint32_t)m_featureInfo.displayName.length();
        res = VmbErrorSuccess;
    }
    else if ( m_featureInfo.displayName.length() <= rnLength )
    {
        std::copy( m_featureInfo.displayName.begin(), m_featureInfo.displayName.end(), pStrDisplayName );
        rnLength = (VmbUint32_t)m_featureInfo.displayName.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType BaseFeature::GetDataType( VmbFeatureDataType &reDataType ) const
{
    reDataType = (VmbFeatureDataType)m_featureInfo.featureDataType;
    
    return VmbErrorSuccess;
}

VmbErrorType BaseFeature::GetFlags( VmbFeatureFlagsType &reFlags ) const
{
    reFlags = (VmbFeatureFlagsType)m_featureInfo.featureFlags;
        
    return VmbErrorSuccess;
}

VmbErrorType BaseFeature::GetCategory( char * const pStrCategory, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrCategory )
    {
        rnLength = (VmbUint32_t)m_featureInfo.category.length();
        res = VmbErrorSuccess;
    }
    else if ( m_featureInfo.category.length() <= rnLength )
    {
        std::copy( m_featureInfo.category.begin(), m_featureInfo.category.end(), pStrCategory );
        rnLength = (VmbUint32_t)m_featureInfo.category.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType BaseFeature::GetPollingTime( VmbUint32_t &rnPollingTime ) const
{
    rnPollingTime = m_featureInfo.pollingTime;
    
    return VmbErrorSuccess;
}

VmbErrorType BaseFeature::GetUnit( char * const pStrUnit, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrUnit )
    {
        rnLength = (VmbUint32_t)m_featureInfo.unit.length();
        res = VmbErrorSuccess;
    }
    else if ( m_featureInfo.unit.length() <= rnLength )
    {
        std::copy( m_featureInfo.unit.begin(), m_featureInfo.unit.end(), pStrUnit );
        rnLength = (VmbUint32_t)m_featureInfo.unit.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType BaseFeature::GetRepresentation( char * const pStrRepresentation, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrRepresentation )
    {
        rnLength = (VmbUint32_t)m_featureInfo.representation.length();
        res = VmbErrorSuccess;
    }
    else if ( m_featureInfo.representation.length() <= rnLength )
    {
        std::copy( m_featureInfo.representation.begin(), m_featureInfo.representation.end(), pStrRepresentation );
        rnLength = (VmbUint32_t)m_featureInfo.representation.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType BaseFeature::GetVisibility( VmbFeatureVisibilityType &reVisibility ) const
{
    reVisibility = (VmbFeatureVisibilityType)m_featureInfo.visibility;

    return VmbErrorSuccess;
}

VmbErrorType BaseFeature::GetToolTip( char * const pStrToolTip, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrToolTip )
    {
        rnLength = (VmbUint32_t)m_featureInfo.tooltip.length();
        res = VmbErrorSuccess;
    }
    else if ( m_featureInfo.tooltip.length() <= rnLength )
    {
        std::copy( m_featureInfo.tooltip.begin(), m_featureInfo.tooltip.end(), pStrToolTip );
        rnLength = (VmbUint32_t)m_featureInfo.tooltip.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType BaseFeature::GetDescription( char * const pStrDescription, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrDescription )
    {
        rnLength = (VmbUint32_t)m_featureInfo.description.length();
        res = VmbErrorSuccess;
    }
    else if ( m_featureInfo.description.length() <= rnLength )
    {
        std::copy( m_featureInfo.description.begin(), m_featureInfo.description.end(), pStrDescription );
        rnLength = (VmbUint32_t)m_featureInfo.description.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType BaseFeature::GetSFNCNamespace( char * const pStrSFNCNamespace, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrSFNCNamespace )
    {
        rnLength = (VmbUint32_t)m_featureInfo.sfncNamespace.length();
        res = VmbErrorSuccess;
    }
    else if ( m_featureInfo.sfncNamespace.length() <= rnLength )
    {
        std::copy( m_featureInfo.sfncNamespace.begin(), m_featureInfo.sfncNamespace.end(), pStrSFNCNamespace );
        rnLength = (VmbUint32_t)m_featureInfo.sfncNamespace.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType BaseFeature::GetAffectedFeatures( FeaturePtr *pAffectedFeatures, VmbUint32_t &rnSize )
{
    VmbError_t res;

    if ( NULL == pAffectedFeatures )
    {
        // Affected features were fetched before
        if ( true == m_pImpl->m_bAffectedFeaturesFetched )
        {
            rnSize = (VmbUint32_t)m_pImpl->m_affectedFeatures.size();

            res = VmbErrorSuccess;
        }
        // Affected features have not been fetched before
        else
        {
            return (VmbErrorType)VmbFeatureListAffected( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), NULL, 0, &rnSize, sizeof(VmbFeatureInfo_t) );
        }
    }
    else
    {
        // Affected features were fetched before
        if ( true == m_pImpl->m_bAffectedFeaturesFetched )
        {
            if ( rnSize < m_pImpl->m_affectedFeatures.size() )
            {
                return VmbErrorMoreData;
            }

            rnSize = (VmbUint32_t)m_pImpl->m_affectedFeatures.size();

            std::copy( m_pImpl->m_affectedFeatures.begin(), m_pImpl->m_affectedFeatures.end(), pAffectedFeatures );

            res = VmbErrorSuccess;
        }
        // Affected features have not been fetched before
        else
        {
            // Check whether the given array size fits
            VmbUint32_t nSize = 0;
            res = VmbFeatureListAffected( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), NULL, 0, &nSize, sizeof(VmbFeatureInfo_t) );

            m_pImpl->m_bAffectedFeaturesFetched = true;

            if ( rnSize < nSize )
            {
                return VmbErrorMoreData;
            }

            rnSize = (VmbUint32_t)nSize;

            if (    VmbErrorSuccess != res
                 || 0 == rnSize )
            {
                return (VmbErrorType)res;
            }

            // Fetch affected features and store them as well as hand them out
            std::vector<VmbFeatureInfo_t> affectedFeatureInfos;
            affectedFeatureInfos.resize( rnSize );

            res = VmbFeatureListAffected( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &affectedFeatureInfos[0], (VmbUint32_t)affectedFeatureInfos.size(), &nSize, sizeof(VmbFeatureInfo_t) );
            
            if ( rnSize < nSize )
            {
                return VmbErrorMoreData;
            }

            rnSize = (VmbUint32_t)nSize;

            for ( VmbUint32_t i=0; i<rnSize; ++i )
            {
                FeaturePtr pFeature;
                res = m_pFeatureContainer->GetFeatureByName( affectedFeatureInfos[i].name, pFeature );
                if ( VmbErrorSuccess != res )
                {
                    m_pImpl->m_affectedFeatures.clear();
                    return (VmbErrorType)res;
                }
                m_pImpl->m_affectedFeatures.push_back( pFeature );
                pAffectedFeatures[i] = m_pImpl->m_affectedFeatures[i];
            }
        }
    }

    return (VmbErrorType)res;
}

VmbErrorType BaseFeature::GetSelectedFeatures( FeaturePtr *pSelectedFeatures, VmbUint32_t &rnSize )
{
    VmbError_t res;

    if ( NULL == pSelectedFeatures )
    {
        // Selected features were fetched before
        if ( true == m_pImpl->m_bSelectedFeaturesFetched )
        {
            rnSize = (VmbUint32_t)m_pImpl->m_selectedFeatures.size();

            res = VmbErrorSuccess;
        }
        // Selected features have not been fetched before
        else
        {
            return (VmbErrorType)VmbFeatureListSelected( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), NULL, 0, &rnSize, sizeof(VmbFeatureInfo_t) );
        }
    }
    else
    {
        // Selected features were fetched before
        if ( true == m_pImpl->m_bSelectedFeaturesFetched )
        {
            if ( rnSize < m_pImpl->m_selectedFeatures.size() )
            {
                return VmbErrorMoreData;
            }

            rnSize = (VmbUint32_t)m_pImpl->m_selectedFeatures.size();

            std::copy( m_pImpl->m_selectedFeatures.begin(), m_pImpl->m_selectedFeatures.end(), pSelectedFeatures );

            res = VmbErrorSuccess;
        }
        // Selected features have not been fetched before
        else
        {
            // Check whether the given array size fits
            VmbUint32_t nSize = 0;
            res = VmbFeatureListSelected( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), NULL, 0, &nSize, sizeof(VmbFeatureInfo_t) );

            m_pImpl->m_bSelectedFeaturesFetched = true;

            if ( rnSize < nSize )
            {
                return VmbErrorMoreData;
            }

            rnSize = (VmbUint32_t)nSize;

            if (    VmbErrorSuccess != res
                 || 0 == rnSize )
            {
                return (VmbErrorType)res;
            }

            // Fetch selected features and store them as well as hand them out
            std::vector<VmbFeatureInfo_t> selectedFeatureInfos;
            selectedFeatureInfos.resize( rnSize );

            res = VmbFeatureListSelected( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &selectedFeatureInfos[0], (VmbUint32_t)selectedFeatureInfos.size(), &nSize, sizeof(VmbFeatureInfo_t) );

            if ( rnSize < nSize )
            {
                return VmbErrorMoreData;
            }

            rnSize = (VmbUint32_t)nSize;

            for ( VmbUint32_t i=0; i<rnSize; ++i )
            {
                FeaturePtr pFeature;
                res = m_pFeatureContainer->GetFeatureByName( selectedFeatureInfos[i].name, pFeature );
                if ( VmbErrorSuccess != res )
                {
                    m_pImpl->m_selectedFeatures.clear();
                    return (VmbErrorType)res;
                }
                m_pImpl->m_selectedFeatures.push_back( pFeature );
                pSelectedFeatures[i] = m_pImpl->m_selectedFeatures[i];
            }
        }
    }

    return (VmbErrorType)res;
}

VmbErrorType BaseFeature::IsReadable( bool &rbIsReadable )
{
    bool bIsWritable = false;

    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }
    
    return (VmbErrorType)VmbFeatureAccessQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rbIsReadable, &bIsWritable );
}

VmbErrorType BaseFeature::IsWritable( bool &rbIsWritable )
{
    bool bIsReadable = false;

    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureAccessQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &bIsReadable, &rbIsWritable );
}

VmbErrorType BaseFeature::IsStreamable( bool &rbIsStreamable ) const
{
    rbIsStreamable = m_featureInfo.isStreamable;

    return VmbErrorSuccess;
}


}} // namespace AVT::VmbAPI
