// Copyright (c) 2024 Fraunhofer IOSB and contributors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the Fraunhofer IOSB nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef CAMERA_ARAVIS2__CAMERA_ARAVIS_NODE_BASE_H_
#define CAMERA_ARAVIS2__CAMERA_ARAVIS_NODE_BASE_H_

// Std
#include <cassert>
#include <map>
#include <string>
#include <utility>
#include <vector>

// Aravis
extern "C"
{
#include "aravis-0.8/arv.h"
}

// ROS
#include <rclcpp/rclcpp.hpp>

// camera_aravis2
#include "camera_aravis2/error.h"

// Macro to assert success of given function
#define CHECK_SUCCESS(expr, logger)                                         \
    if (!expr)                                                              \
    {                                                                       \
        RCLCPP_FATAL(logger, "%s:%i: Assertion on success of '%s' failed.", \
                     __FILE__, __LINE__, #expr);                            \
        return;                                                             \
    }

namespace camera_aravis2
{

class CameraAravisNodeBase : public rclcpp::Node
{
    //--- METHOD DECLARATION ---//

  public:
    /**
     * @brief Initialization constructor
     *
     * @param[in] name Node name.
     * @param[in] options Node options.
     */
    explicit CameraAravisNodeBase(const std::string& name,
                                  const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    /**
     * @brief Default destructor.
     *
     */
    virtual ~CameraAravisNodeBase();

    /**
     * @brief Returns true, if node is is initialized. False, otherwise.
     */
    bool isInitialized() const;

    /**
     * @brief List available camera devices.
     *
     * @return Number of available devices.
     */
    uint listAvailableCameraDevices() const;

  protected:
    /**
     * @brief Set the up launch parameters.
     */
    virtual void setupParameters();

    /**
     * @brief Discover attached camera devices found by Aravis and open device specified by guid.
     *
     * @return True if successful. False, otherwise.
     */
    [[nodiscard]] bool discoverAndOpenCameraDevice();

    /**
     * @brief Get nested parameter with 'param_name' with 'parent_name' as parent.
     *
     * @param[in] parent_name Name of the parent parameter.
     * @param[in] param_name Name of the nested parameter underneath the parent.
     * @param[out] param_value Parameter value.
     * @return True if parameter is found in 'parameter_overrides_' and, thus, given by the user.
     * False otherwise.
     */
    [[nodiscard]] bool getNestedParameter(const std::string& parent_name,
                                          const std::string& param_name,
                                          rclcpp::ParameterValue& param_value) const;

    /**
     * @brief Get list of nested parameter with 'param_name' with 'parent_name' as parent.
     *
     * @param[in] parent_name Name of the parent parameter.
     * @param[in] param_name Name of the nested parameter underneath the parent. If 'param_name' is
     * left empty, all nested parameters will be searched for underneath 'parent_name'.
     * @param[out] param_values List of parameter values associated with feature names.
     * @return True if sub category is found in 'parameter_overrides_' and, thus, given by the user.
     * False otherwise.
     */
    [[nodiscard]] bool getNestedParameterList(
      const std::string& parent_name,
      const std::string& param_name,
      std::vector<std::pair<std::string, rclcpp::ParameterValue>>& param_values) const;

    /**
     * @brief Get feature value if it is available.
     *
     * @tparam T Type of feature value.
     * @param[in] feature_name Name of feature.
     * @param[out] value Value of feature name. If method returns false, value is unchanged.
     * @return Returns true if successful, false otherwise.
     */
    template <typename T>
    bool getFeatureValue(const std::string& feature_name, T& value) const;

    /**
     * @brief Set feature value if it is available.
     *
     * @tparam T Type of feature value.
     * @param[in] feature_name Name of feature.
     * @param[in] value Value to set.
     * @return Returns true if successful, false otherwise.
     */
    template <typename T>
    bool setFeatureValue(const std::string& feature_name, const T& value) const;

    /**
     * @overload
     * @brief Set feature value if it is available.
     *
     * This will truncate the value at the given minimum and maximum bound prior to setting.
     *
     * @tparam T Type of feature value.
     * @param[in] feature_name Name of feature.
     * @param[in] value Value to set.
     * @param[in] min Minimum bound of the value that is to be set.
     * @param[in] max Maximum bound of the value that is to be set.
     * @return Returns true if successful, false otherwise.
     */
    template <typename T>
    bool setFeatureValue(const std::string& feature_name, const T& value,
                         const T& min, const T& max) const;

    /**
     * @brief Set bounded feature value if it is available.
     *
     * This will first get the bounds and then truncate the value accordingly before setting it.
     *
     * @tparam T Type of feature value.
     * @param[in] feature_name Name of feature.
     * @param[in] value Value to set.
     * @param[out] min Optional pointer to variable in which to store minimum bound.
     * @param[out] max Optional pointer to variable in which to store maximum bound.
     * @return Returns true if successful, false otherwise.
     */
    template <typename T>
    bool setBoundedFeatureValue(const std::string& feature_name, const T& value,
                                T* min = nullptr, T* max = nullptr) const;

    /**
     * @brief Set feature from parameter value if it is available.
     *
     * This will first check if the parameter is an array type. If so, it will use 'idx' to access
     * the parameter. If the given index outside of the range, the last value of the list is used.
     *
     * @tparam T Type of feature value.
     * @param[in] feature_name Name of feature.
     * @param[in] parameter_value Specified parameter values.
     * @param[in] idx Index of parameter value that is to be set. Only used if parameter values are
     * given as array.
     * @return Returns true if successful, false otherwise.
     */
    template <typename T>
    bool setFeatureValueFromParameter(const std::string& feature_name,
                                      const rclcpp::ParameterValue& parameter_value,
                                      const uint& idx = 0) const;

    /**
     * @overload
     * @brief Set feature from parameter value if it is available.
     *
     * This will first check if the parameter is an array type. If so, it will use 'idx' to access
     * the parameter. If the given index outside of the range, the last value of the list is used.
     *
     * This will also truncate the value at the given minimum and maximum bound prior to setting.
     *
     * @tparam T Type of feature value.
     * @param[in] feature_name Name of feature.
     * @param[in] parameter_value Specified parameter values.
     * @param[in] min Minimum bound of the value that is to be set.
     * @param[in] max Maximum bound of the value that is to be set.
     * @param[in] idx Index of parameter value that is to be set. Only used if parameter values are
     * given as array.
     * @return Returns true if successful, false otherwise.
     */
    template <typename T>
    bool setFeatureValueFromParameter(const std::string& feature_name,
                                      const rclcpp::ParameterValue& parameter_value,
                                      const T& min, const T& max,
                                      const uint& idx = 0) const;

    /**
     * @brief Set features from list of parameter values.
     *
     * This will loop through the list of parameter value associated with the feature names and call
     * #setFeatureValueFromParameter accordingly.
     *
     * @param[in] param_values List of parameter values associated with feature names.
     * @param[in] idx Index of parameter value that is to be set. Only used if parameter values are
     * given as array.
     * @return Returns true if successful, false otherwise.
     */
    bool setFeatureValuesFromParameterList(
      const std::vector<std::pair<std::string, rclcpp::ParameterValue>>& param_values,
      const uint& idx = 0) const;

    /**
     * @brief Set bounded feature from parameter value if it is available.
     *
     * If the parameter is available, this will first check if the parameter is an array type. If
     * so, it will use 'idx' to access the parameter. If the given index outside of the range, the
     * last value of the list is used.
     *
     * Values outside of the range specified by 'min' and 'max', will be truncated to the range.
     *
     * @tparam T Type of feature value.
     * @param[in] feature_name Name of feature.
     * @param[in] parameter_value Specified parameter values.
     * @param[out] min Optional pointer to variable in which to store minimum bound.
     * @param[out] max Optional pointer to variable in which to store maximum bound.
     * @param[in] idx Index of parameter value that is to be set. Only used if parameter values are
     * given as array.
     * @return Returns true if successful, false otherwise.
     */
    template <typename T>
    bool setBoundedFeatureValueFromParameter(const std::string& feature_name,
                                             const rclcpp::ParameterValue& parameter_value,
                                             T* min = nullptr, T* max = nullptr,
                                             const uint& idx = 0) const;

    /**
     * @brief Check if parameter value is equal to given test value.
     *
     * @tparam T Type of test value.
     * @param[in] parameter_value Specified parameter values.
     * @param[in] test_value Test value at which the parameter value is to be checked against.
     * @param[in] idx Index of parameter value that is to be set. Only used if parameter values are
     * given as array.
     * @return Returns true if equal, false otherwise.
     */
    template <typename T>
    bool isParameterValueEqualTo(const rclcpp::ParameterValue& parameter_value,
                                 const T& test_value,
                                 const uint& idx = 0) const;

    /**
     * @brief Execute command with given feature name.
     *
     * @param[in] feature_name Name of command feature.
     * @return Returns true if successful, false otherwise (e.g. command feature is not available).
     */
    bool executeCommand(const std::string& feature_name) const;

    //--- FUNCTION DECLARATION ---//

  protected:
    /**
     * @brief Check if given string is an IP Adress.
     */
    static bool isIpAddress(const std::string& str);

    /**
     * @brief Construct GUID string of given camera, using vendor name, model name and
     * device serial number.
     *
     * @return GUID in the format: <vendor_name>-<model_name>-<device_sn | device_id>.
     */
    static std::string constructCameraGuidStr(ArvCamera* p_cam);

    /**
     * @brief Handle 'control-lost' signal emitted by aravis.
     *
     * @param[in] p_device Pointer to aravis device.
     * @param[in] p_user_data Pointer to associated user data.
     */
    static void handleControlLostSignal(ArvDevice* p_device, gpointer p_user_data);

    //--- MEMBER DECLARATION ---//

  protected:
    /// Flag indicating if node is initialized.
    bool is_initialized_;

    /// Logger object of node.
    rclcpp::Logger logger_;

    /// Pointer to Aravis device.
    ArvDevice* p_device_;

    /// Pointer to Aravis camera.
    ArvCamera* p_camera_;

    /// GUID of camera.
    std::string guid_;

    /// List of parameter overrides, including parameters that have not been declared.
    std::map<std::string, rclcpp::ParameterValue> parameter_overrides_;

    /// Flag indicating verbose output.
    bool is_verbose_enable_;
};

}  // namespace camera_aravis2

#endif  // CAMERA_ARAVIS2__CAMERA_ARAVIS_NODE_BASE_H_
