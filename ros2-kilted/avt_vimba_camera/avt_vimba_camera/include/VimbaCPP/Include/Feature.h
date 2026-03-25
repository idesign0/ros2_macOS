/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Feature.h

  Description:  Definition of base class AVT::VmbAPI::Feature.
                This class wraps every call to BaseFeature resp. its concrete
                subclass. That way  polymorphism is hidden away from the user.
                

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

#ifndef AVT_VMBAPI_FEATURE_H
#define AVT_VMBAPI_FEATURE_H

#include <vector>
#include <map>

#include <VimbaC/Include/VimbaC.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/SharedPointerDefines.h>
#include <VimbaCPP/Include/IFeatureObserver.h>
#include <VimbaCPP/Include/EnumEntry.h>

namespace AVT {
namespace VmbAPI {

class BaseFeature;

typedef std::vector<FeaturePtr> FeaturePtrVector;
typedef std::map<std::string, FeaturePtr> FeaturePtrMap;

class Feature
{
  public:
    Feature( const VmbFeatureInfo_t *pFeatureInfo, FeatureContainer *pFeatureContainer );
    virtual ~Feature();

    //
    // Method:      GetValue()
    //
    // Purpose:     Queries the value of a feature of type VmbInt64
    //
    // Parameters:
    //
    // [out]    VmbInt64_t&     value       The feature's value
    //
    IMEXPORT    VmbErrorType GetValue( VmbInt64_t &value ) const;
    
    //
    // Method:      GetValue()
    //
    // Purpose:     Queries the value of a feature of type double
    //
    // Parameters:
    //
    // [out]    double&     value       The feature's value
    //
    IMEXPORT    VmbErrorType GetValue( double &value ) const;

    //
    // Method:      GetValue()
    //
    // Purpose:     Queries the value of a feature of type string
    //
    // Parameters:
    //
    // [out]    std::string&    value       The feature's value
    //
    // Details:     When an empty string is returned, its size
    //              indicates the maximum length
    VmbErrorType GetValue( std::string &value ) const;

    //
    // Method:      GetValue()
    //
    // Purpose:     Queries the value of a feature of type bool
    //
    // Parameters:
    //
    // [out]    bool&       value       The feature's value
    //
    IMEXPORT    VmbErrorType GetValue( bool &value ) const;

    //
    // Method:      GetValue()
    //
    // Purpose:     Queries the value of a feature of type UcharVector
    //
    // Parameters:
    //
    // [out]    UcharVector&    value       The feature's value
    //
    VmbErrorType GetValue( UcharVector &value ) const;

    //
    // Method:      GetValue()
    //
    // Purpose:     Queries the value of a feature of type const UcharVector
    //
    // Parameters:
    //
    // [out]    UcharVector&      value       The feature's value
    // [out]    VmbUint32_t&            sizeFilled  The number of actually received values
    //
    VmbErrorType GetValue( UcharVector &value, VmbUint32_t &sizeFilled ) const;

    //
    // Method:      GetValues()
    //
    // Purpose:     Queries the values of a feature of type Int64Vector
    //
    // Parameters:
    //
    // [out]    Int64Vector&    values       The feature's values
    //
    VmbErrorType GetValues( Int64Vector &values );

    //
    // Method:      GetValues()
    //
    // Purpose:     Queries the values of a feature of type StringVector
    //
    // Parameters:
    //
    // [out]    StringVector&    values       The feature's values
    //
    VmbErrorType GetValues( StringVector &values );

    //
    // Method:      GetEntry()
    //
    // Purpose:     Queries a single enum entry of a feature of type Enumeration
    //
    // Parameters:
    //
    // [out]    EnumEntry&    entry       An enum feature's enum entry
    // [in ]    const char*   pEntryName  The name of the enum entry
    //
    IMEXPORT    VmbErrorType GetEntry( EnumEntry &entry, const char *pEntryName ) const;

    //
    // Method:      GetEntries()
    //
    // Purpose:     Queries all enum entries of a feature of type Enumeration
    //
    // Parameters:
    //
    // [out]    EnumEntryVector&   entries       An enum feature's enum entries
    //
    VmbErrorType GetEntries( EnumEntryVector &entries );

    //
    // Method:      GetRange()
    //
    // Purpose:     Queries the range of a feature of type double
    //
    // Parameters:
    //
    // [out]    double&    minimum   The feature's min value
    // [out]    double&    maximum   The feature's max value
    //
    IMEXPORT    VmbErrorType GetRange( double &minimum, double &maximum ) const;

    //
    // Method:      GetRange()
    //
    // Purpose:     Queries the range of a feature of type VmbInt64
    //
    // Parameters:
    //
    // [out]    VmbInt64_t&    minimum   The feature's min value
    // [out]    VmbInt64_t&    maximum   The feature's max value
    //
    IMEXPORT    VmbErrorType GetRange( VmbInt64_t &minimum, VmbInt64_t &maximum ) const;

    //
    // Method:      SetValue()
    //
    // Purpose:     Sets the value of a feature of type VmbInt32
    //
    // Parameters:
    //
    // [in ]    const VmbInt32_t&    value       The feature's value
    //
    IMEXPORT    VmbErrorType SetValue( const VmbInt32_t &value );

    //
    // Method:      SetValue()
    //
    // Purpose:     Sets the value of a feature of type VmbInt64
    //
    // Parameters:
    //
    // [in ]    const VmbInt64_t&   value       The feature's value
    //
    IMEXPORT    VmbErrorType SetValue( const VmbInt64_t &value );

    //
    // Method:      SetValue()
    //
    // Purpose:     Sets the value of a feature of type double
    //
    // Parameters:
    //
    // [in ]    const double&    value       The feature's value
    //
    IMEXPORT    VmbErrorType SetValue( const double &value );

    //
    // Method:      SetValue()
    //
    // Purpose:     Sets the value of a feature of type char*
    //
    // Parameters:
    //
    // [in ]    const char*    pValue       The feature's value
    //
    IMEXPORT    VmbErrorType SetValue( const char *pValue );

    //
    // Method:      SetValue()
    //
    // Purpose:     Sets the value of a feature of type bool
    //
    // Parameters:
    //
    // [in ]    bool    value       The feature's value
    //
    IMEXPORT    VmbErrorType SetValue( bool value );

    //
    // Method:      SetValue()
    //
    // Purpose:     Sets the value of a feature of type UcharVector
    //
    // Parameters:
    //
    // [in ]    const UcharVector&    value       The feature's value
    //
    VmbErrorType SetValue( const UcharVector &value );

    // Method:      HasIncrement()
    //
    // Purpose:     Gets the support state increment of a feature
    //
    // Parameters:
    //
    // [out]    VmbBool_t&    incrementsupported       The feature's increment support state
    //
    IMEXPORT    VmbErrorType HasIncrement( VmbBool_t &incrementSupported ) const;


    //
    // Method:      GetIncrement()
    //
    // Purpose:     Gets the increment of a feature of type VmbInt64
    //
    // Parameters:
    //
    // [out]    VmbInt64_t&    increment       The feature's increment
    //
    IMEXPORT    VmbErrorType GetIncrement( VmbInt64_t &increment ) const;

    // Method:      GetIncrement()
    //
    // Purpose:     Gets the increment of a feature of type double
    //
    // Parameters:
    //
    // [out]    double&    increment       The feature's increment
    //
    IMEXPORT    VmbErrorType GetIncrement( double &increment ) const;

    //
    // Method:      IsValueAvailable()
    //
    // Purpose:     Indicates whether an existing enumeration value is currently available.
    //              An enumeration value might not be selectable due to the camera's
    //              current configuration.
    //
    // Parameters:
    //
    // [in ]        const char*     pValue      The enumeration value as string
    // [out]        bool&           available   True when the given value is available
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorInvalidValue:  If the given value is not a valid enumeration value for this enum
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorInvalidAccess: Operation is invalid with the current access mode
    //  - VmbErrorWrongType:     The feature is not an enumeration
    //
    IMEXPORT    VmbErrorType IsValueAvailable( const char *pValue, bool &available ) const;

    //
    // Method:      IsValueAvailable()
    //
    // Purpose:     Indicates whether an existing enumeration value is currently available.
    //              An enumeration value might not be selectable due to the camera's
    //              current configuration.
    //
    // Parameters:
    //
    // [in ]        const VmbInt64_t    value       The enumeration value as int
    // [out]        bool&               available   True when the given value is available
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorInvalidValue:  If the given value is not a valid enumeration value for this enum
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorInvalidAccess: Operation is invalid with the current access mode
    //  - VmbErrorWrongType:     The feature is not an enumeration
    //
    IMEXPORT    VmbErrorType IsValueAvailable( const VmbInt64_t value, bool &available ) const;

    //
    // Method:      RunCommand()
    //
    // Purpose:     Executes a feature of type Command
    //
    IMEXPORT    VmbErrorType RunCommand();

    //
    // Method:      IsCommandDone()
    //
    // Purpose:     Indicates whether the execution of a feature of type Command has finished
    //
    // Parameters:
    //
    // [out]    bool&    isDone     True when execution has finished
    //
    IMEXPORT    VmbErrorType IsCommandDone( bool &isDone ) const;

    //
    // Method:      GetName()
    //
    // Purpose:     Queries a feature's name
    //
    // Parameters:
    //
    // [out]    std::string&    name    The feature's name
    //
    VmbErrorType GetName( std::string &name ) const;

    //
    // Method:      GetDisplayName()
    //
    // Purpose:     Queries a feature's display name
    //
    // Parameters:
    //
    // [out]    std::string&    displayName    The feature's display name
    //
    VmbErrorType GetDisplayName( std::string &displayName ) const;

    //
    // Method:      GetDataType()
    //
    // Purpose:     Queries a feature's type
    //
    // Parameters:
    //
    // [out]    VmbFeatureDataType&    dataType    The feature's type
    //
    IMEXPORT    VmbErrorType GetDataType( VmbFeatureDataType &dataType ) const;

    //
    // Method:      GetFlags()
    //
    // Purpose:     Queries a feature's access status
    //
    // Parameters:
    //
    // [out]    VmbFeatureFlagsType&    flags    The feature's access status
    //    
    IMEXPORT    VmbErrorType GetFlags( VmbFeatureFlagsType &flags ) const;

    //
    // Method:      GetCategory()
    //
    // Purpose:     Queries a feature's category in the feature tress
    //
    // Parameters:
    //
    // [out]    std::string&    category    The feature's position in the feature tree
    //    
    VmbErrorType GetCategory( std::string &category ) const;

    //
    // Method:      GetPollingTime()
    //
    // Purpose:     Queries a feature's polling time
    //
    // Parameters:
    //
    // [out]    VmbUint32_t&    pollingTime    The interval to poll the feature
    //    
    IMEXPORT    VmbErrorType GetPollingTime( VmbUint32_t &pollingTime ) const;

    //
    // Method:      GetUnit()
    //
    // Purpose:     Queries a feature's unit
    //
    // Parameters:
    //
    // [out]    std::string&    unit    The feature's unit
    //
    VmbErrorType GetUnit( std::string &unit ) const;

    //
    // Method:      GetRepresentation()
    //
    // Purpose:     Queries a feature's representation
    //
    // Parameters:
    //
    // [out]    std::string&    representation    The feature's representation
    //
    VmbErrorType GetRepresentation( std::string &representation ) const;

    //
    // Method:      GetVisibility()
    //
    // Purpose:     Queries a feature's visibility
    //
    // Parameters:
    //
    // [out]    VmbFeatureVisibilityType&    visibility    The feature's visibility
    //
    IMEXPORT    VmbErrorType GetVisibility( VmbFeatureVisibilityType &visibility ) const;

    //
    // Method:      GetToolTip()
    //
    // Purpose:     Queries a feature's tool tip to display in the GUI
    //
    // Parameters:
    //
    // [out]    std::string&    toolTip    The feature's tool tip
    //
    VmbErrorType GetToolTip( std::string &toolTip ) const;

    //
    // Method:      GetDescription()
    //
    // Purpose:     Queries a feature's description
    //
    // Parameters:
    //
    // [out]    std::string&    description    The feature'sdescription
    //
    VmbErrorType GetDescription( std::string &description ) const;

    //
    // Method:      GetSFNCNamespace()
    //
    // Purpose:     Queries a feature's Standard Feature Naming Convention namespace
    //
    // Parameters:
    //
    // [out]    std::string&    sFNCNamespace    The feature's SFNC namespace
    //
    VmbErrorType GetSFNCNamespace( std::string &sFNCNamespace ) const;

    //
    // Method:      GetAffectedFeatures()
    //
    // Purpose:     Queries the feature's that are dependent from the current feature
    //
    // Parameters:
    //
    // [out]    FeaturePtrVector&    affectedFeatures    The features that get invalidated through the current feature
    //
    VmbErrorType GetAffectedFeatures( FeaturePtrVector &affectedFeatures );

    //
    // Method:      GetSelectedFeatures()
    //
    // Purpose:     Gets the features that get selected by the current feature
    //
    // Parameters:
    //
    // [out]    FeaturePtrVector&    selectedFeatures    The selected features
    //
    VmbErrorType GetSelectedFeatures( FeaturePtrVector &selectedFeatures );

    //
    // Method:      IsReadable()
    //
    // Purpose:     Queries the read access status of a feature
    //
    // Parameters:
    //
    // [out]    bool&    isReadable    True when feature can be read
    //
    IMEXPORT    VmbErrorType IsReadable( bool &isReadable );

    //
    // Method:      IsWritable()
    //
    // Purpose:     Queries the write access status of a feature
    //
    // Parameters:
    //
    // [out]    bool&    isWritable    True when feature can be written
    //
    IMEXPORT    VmbErrorType IsWritable( bool &isWritable );

    //
    // Method:      IsStreamable()
    //
    // Purpose:     Queries whether a feature's value can be transferred as a stream
    //
    // Parameters:
    //
    // [out]    bool&    isStreamable    True when streamable
    //
    IMEXPORT    VmbErrorType IsStreamable( bool &isStreamable ) const;

    //
    // Method:      RegisterObserver()
    //
    // Purpose:     Registers an observer that notifies the application whenever a features value changes
    //
    // Parameters:
    //
    // [out]    const IFeatureObserverPtr&    pObserver    The observer to be registered
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorBadParameter:  "pObserver" is NULL.
    //
    IMEXPORT    VmbErrorType RegisterObserver( const IFeatureObserverPtr &pObserver );

    //
    // Method:      UnregisterObserver()
    //
    // Purpose:     Unregisters an observer
    //
    // Parameters:
    //
    // [out]    const IFeatureObserverPtr&    pObserver    The observer to be unregistered
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorBadParameter:  "pObserver" is NULL.
    //
    IMEXPORT    VmbErrorType UnregisterObserver( const IFeatureObserverPtr &pObserver );

    void ResetFeatureContainer();

  private:
    BaseFeature *m_pImpl;

    // No default ctor
    Feature();
    // No copy ctor
    Feature( const Feature& );
    // No assignment operator
    Feature& operator=( const Feature& );

    // Array functions to pass data across DLL boundaries
    IMEXPORT    VmbErrorType GetValue( char * const pValue, VmbUint32_t &length ) const;
    IMEXPORT    VmbErrorType GetValue( VmbUchar_t *pValue, VmbUint32_t &size, VmbUint32_t &sizeFilled ) const;
    IMEXPORT    VmbErrorType GetValues( const char **pValues, VmbUint32_t &size );
    IMEXPORT    VmbErrorType GetValues( VmbInt64_t *pValues, VmbUint32_t &Size );

    IMEXPORT    VmbErrorType GetEntries( EnumEntry *pEnumEntries, VmbUint32_t &size );

    IMEXPORT    VmbErrorType SetValue( const VmbUchar_t *pValue, VmbUint32_t size );

    IMEXPORT    VmbErrorType GetName( char * const pName, VmbUint32_t &length ) const;
    IMEXPORT    VmbErrorType GetDisplayName( char * const pDisplayName, VmbUint32_t &length ) const;
    IMEXPORT    VmbErrorType GetCategory( char * const pCategory, VmbUint32_t &length ) const;
    IMEXPORT    VmbErrorType GetUnit( char * const pUnit, VmbUint32_t &length ) const;
    IMEXPORT    VmbErrorType GetRepresentation( char * const pRepresentation, VmbUint32_t &length ) const;
    IMEXPORT    VmbErrorType GetToolTip( char * const pToolTip, VmbUint32_t &length ) const;
    IMEXPORT    VmbErrorType GetDescription( char * const pDescription, VmbUint32_t &length ) const;
    IMEXPORT    VmbErrorType GetSFNCNamespace( char * const pSFNCNamespace, VmbUint32_t &length ) const;
    IMEXPORT    VmbErrorType GetAffectedFeatures( FeaturePtr *pAffectedFeatures, VmbUint32_t &nSize );
    IMEXPORT    VmbErrorType GetSelectedFeatures( FeaturePtr *pSelectedFeatures, VmbUint32_t &nSize );
};

#include <VimbaCPP/Include/Feature.hpp>

}} // namespace AVT::VmbAPI

#endif
