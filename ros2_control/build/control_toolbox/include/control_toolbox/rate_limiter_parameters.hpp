// auto-generated DO NOT EDIT

#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <mutex>
#include <rclcpp/node.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <rclcpp/logger.hpp>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <parameter_traits/parameter_traits.hpp>

#include <rsl/static_string.hpp>
#include <rsl/static_vector.hpp>
#include <rsl/parameter_validators.hpp>

#include "custom_validators.hpp"


namespace rate_limiter {

// Use validators from RSL
using rsl::unique;
using rsl::subset_of;
using rsl::fixed_size;
using rsl::size_gt;
using rsl::size_lt;
using rsl::not_empty;
using rsl::element_bounds;
using rsl::lower_element_bounds;
using rsl::upper_element_bounds;
using rsl::bounds;
using rsl::lt;
using rsl::gt;
using rsl::lt_eq;
using rsl::gt_eq;
using rsl::one_of;
using rsl::to_parameter_result_msg;

// temporarily needed for backwards compatibility for custom validators
using namespace parameter_traits;

template <typename T>
[[nodiscard]] auto to_parameter_value(T value) {
    return rclcpp::ParameterValue(value);
}

template <size_t capacity>
[[nodiscard]] auto to_parameter_value(rsl::StaticString<capacity> const& value) {
    return rclcpp::ParameterValue(rsl::to_string(value));
}

template <typename T, size_t capacity>
[[nodiscard]] auto to_parameter_value(rsl::StaticVector<T, capacity> const& value) {
    return rclcpp::ParameterValue(rsl::to_vector(value));
}
    struct Params {
        double sampling_interval;
        double max_value = std::numeric_limits<double>::quiet_NaN();
        double min_value = std::numeric_limits<double>::quiet_NaN();
        double max_first_derivative_pos = std::numeric_limits<double>::quiet_NaN();
        double min_first_derivative_pos = std::numeric_limits<double>::quiet_NaN();
        double max_first_derivative_neg = std::numeric_limits<double>::quiet_NaN();
        double min_first_derivative_neg = std::numeric_limits<double>::quiet_NaN();
        double max_second_derivative = std::numeric_limits<double>::quiet_NaN();
        double min_second_derivative = std::numeric_limits<double>::quiet_NaN();
        // for detecting if the parameter struct has been updated
        rclcpp::Time __stamp;
    };
    struct StackParams {
        double sampling_interval;
        double max_value = std::numeric_limits<double>::quiet_NaN();
        double min_value = std::numeric_limits<double>::quiet_NaN();
        double max_first_derivative_pos = std::numeric_limits<double>::quiet_NaN();
        double min_first_derivative_pos = std::numeric_limits<double>::quiet_NaN();
        double max_first_derivative_neg = std::numeric_limits<double>::quiet_NaN();
        double min_first_derivative_neg = std::numeric_limits<double>::quiet_NaN();
        double max_second_derivative = std::numeric_limits<double>::quiet_NaN();
        double min_second_derivative = std::numeric_limits<double>::quiet_NaN();
    };

  class ParamListener{
  public:
    // throws rclcpp::exceptions::InvalidParameterValueException on initialization if invalid parameter are loaded
    ParamListener(rclcpp::Node::SharedPtr node, std::string const& prefix = "")
    : ParamListener(node->get_node_parameters_interface(), node->get_logger(), prefix) {}

    ParamListener(rclcpp_lifecycle::LifecycleNode::SharedPtr node, std::string const& prefix = "")
    : ParamListener(node->get_node_parameters_interface(), node->get_logger(), prefix) {}

    ParamListener(const std::shared_ptr<rclcpp::node_interfaces::NodeParametersInterface>& parameters_interface,
                  std::string const& prefix = "")
    : ParamListener(parameters_interface, rclcpp::get_logger("rate_limiter"), prefix) {
      RCLCPP_DEBUG(logger_, "ParameterListener: Not using node logger, recommend using other constructors to use a node logger");
    }

    ParamListener(const std::shared_ptr<rclcpp::node_interfaces::NodeParametersInterface>& parameters_interface,
                  rclcpp::Logger logger, std::string const& prefix = "") {
      logger_ = std::move(logger);
      prefix_ = prefix;
      if (!prefix_.empty() && prefix_.back() != '.') {
        prefix_ += ".";
      }

      parameters_interface_ = parameters_interface;
      declare_params();
      auto update_param_cb = [this](const std::vector<rclcpp::Parameter> &parameters){return this->update(parameters);};
      handle_ = parameters_interface_->add_on_set_parameters_callback(update_param_cb);
      clock_ = rclcpp::Clock();
    }

    Params get_params() const{
      std::lock_guard<std::mutex> lock(mutex_);
      return params_;
    }

    bool try_get_params(Params & params_in) const {
      if (mutex_.try_lock()) {
        if (const bool is_old = params_in.__stamp != params_.__stamp; is_old) {
          params_in = params_;
        }
        mutex_.unlock();
        return true;
      }
      return false;
    }

    bool is_old(Params const& other) const {
      std::lock_guard<std::mutex> lock(mutex_);
      return params_.__stamp != other.__stamp;
    }

    StackParams get_stack_params() {
      Params params = get_params();
      StackParams output;
      output.sampling_interval = params.sampling_interval;
      output.max_value = params.max_value;
      output.min_value = params.min_value;
      output.max_first_derivative_pos = params.max_first_derivative_pos;
      output.min_first_derivative_pos = params.min_first_derivative_pos;
      output.max_first_derivative_neg = params.max_first_derivative_neg;
      output.min_first_derivative_neg = params.min_first_derivative_neg;
      output.max_second_derivative = params.max_second_derivative;
      output.min_second_derivative = params.min_second_derivative;

      return output;
    }

    void refresh_dynamic_parameters() {
      auto updated_params = get_params();
      // TODO remove any destroyed dynamic parameters

      // declare any new dynamic parameters
      rclcpp::Parameter param;

    }

    rcl_interfaces::msg::SetParametersResult update(const std::vector<rclcpp::Parameter> &parameters) {
      auto updated_params = get_params();

      for (const auto &param: parameters) {
        if (param.get_name() == (prefix_ + "sampling_interval")) {
            if(auto validation_result = gt<double>(param, 0.0);
              !validation_result) {
                return rsl::to_parameter_result_msg(validation_result);
            }
            updated_params.sampling_interval = param.as_double();
            RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
        }
        if (param.get_name() == (prefix_ + "max_value")) {
            if(auto validation_result = control_filters::gt_eq_or_nan<double>(param, 0.0);
              !validation_result) {
                return rsl::to_parameter_result_msg(validation_result);
            }
            updated_params.max_value = param.as_double();
            RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
        }
        if (param.get_name() == (prefix_ + "min_value")) {
            if(auto validation_result = control_filters::lt_eq_or_nan<double>(param, 0.0);
              !validation_result) {
                return rsl::to_parameter_result_msg(validation_result);
            }
            updated_params.min_value = param.as_double();
            RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
        }
        if (param.get_name() == (prefix_ + "max_first_derivative_pos")) {
            if(auto validation_result = control_filters::gt_eq_or_nan<double>(param, 0.0);
              !validation_result) {
                return rsl::to_parameter_result_msg(validation_result);
            }
            updated_params.max_first_derivative_pos = param.as_double();
            RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
        }
        if (param.get_name() == (prefix_ + "min_first_derivative_pos")) {
            if(auto validation_result = control_filters::lt_eq_or_nan<double>(param, 0.0);
              !validation_result) {
                return rsl::to_parameter_result_msg(validation_result);
            }
            updated_params.min_first_derivative_pos = param.as_double();
            RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
        }
        if (param.get_name() == (prefix_ + "max_first_derivative_neg")) {
            if(auto validation_result = control_filters::gt_eq_or_nan<double>(param, 0.0);
              !validation_result) {
                return rsl::to_parameter_result_msg(validation_result);
            }
            updated_params.max_first_derivative_neg = param.as_double();
            RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
        }
        if (param.get_name() == (prefix_ + "min_first_derivative_neg")) {
            if(auto validation_result = control_filters::lt_eq_or_nan<double>(param, 0.0);
              !validation_result) {
                return rsl::to_parameter_result_msg(validation_result);
            }
            updated_params.min_first_derivative_neg = param.as_double();
            RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
        }
        if (param.get_name() == (prefix_ + "max_second_derivative")) {
            if(auto validation_result = control_filters::gt_eq_or_nan<double>(param, 0.0);
              !validation_result) {
                return rsl::to_parameter_result_msg(validation_result);
            }
            updated_params.max_second_derivative = param.as_double();
            RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
        }
        if (param.get_name() == (prefix_ + "min_second_derivative")) {
            if(auto validation_result = control_filters::lt_eq_or_nan<double>(param, 0.0);
              !validation_result) {
                return rsl::to_parameter_result_msg(validation_result);
            }
            updated_params.min_second_derivative = param.as_double();
            RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
        }
      }

      updated_params.__stamp = clock_.now();
      update_internal_params(updated_params);
      return rsl::to_parameter_result_msg({});
    }

    void declare_params(){
      auto updated_params = get_params();
      // declare all parameters and give default values to non-required ones
      if (!parameters_interface_->has_parameter(prefix_ + "sampling_interval")) {
          rcl_interfaces::msg::ParameterDescriptor descriptor;
          descriptor.description = "Sampling interval in seconds";
          descriptor.read_only = false;
          descriptor.floating_point_range.resize(1);
          descriptor.floating_point_range.at(0).from_value = 0.0;
          descriptor.floating_point_range.at(0).to_value = std::numeric_limits<double>::max();
          auto parameter = rclcpp::ParameterType::PARAMETER_DOUBLE;
          parameters_interface_->declare_parameter(prefix_ + "sampling_interval", parameter, descriptor);
      }
      if (!parameters_interface_->has_parameter(prefix_ + "max_value")) {
          rcl_interfaces::msg::ParameterDescriptor descriptor;
          descriptor.description = "Maximum value, e.g. [m/s]";
          descriptor.read_only = false;
          auto parameter = to_parameter_value(updated_params.max_value);
          parameters_interface_->declare_parameter(prefix_ + "max_value", parameter, descriptor);
      }
      if (!parameters_interface_->has_parameter(prefix_ + "min_value")) {
          rcl_interfaces::msg::ParameterDescriptor descriptor;
          descriptor.description = "Minimum value, e.g. [m/s]";
          descriptor.read_only = false;
          auto parameter = to_parameter_value(updated_params.min_value);
          parameters_interface_->declare_parameter(prefix_ + "min_value", parameter, descriptor);
      }
      if (!parameters_interface_->has_parameter(prefix_ + "max_first_derivative_pos")) {
          rcl_interfaces::msg::ParameterDescriptor descriptor;
          descriptor.description = "Maximum value of the first derivative if **value** is positive, e.g. [m/s^2]";
          descriptor.read_only = false;
          auto parameter = to_parameter_value(updated_params.max_first_derivative_pos);
          parameters_interface_->declare_parameter(prefix_ + "max_first_derivative_pos", parameter, descriptor);
      }
      if (!parameters_interface_->has_parameter(prefix_ + "min_first_derivative_pos")) {
          rcl_interfaces::msg::ParameterDescriptor descriptor;
          descriptor.description = "Minimum value of the first derivative if **value** is positive, e.g. [m/s^2]";
          descriptor.read_only = false;
          auto parameter = to_parameter_value(updated_params.min_first_derivative_pos);
          parameters_interface_->declare_parameter(prefix_ + "min_first_derivative_pos", parameter, descriptor);
      }
      if (!parameters_interface_->has_parameter(prefix_ + "max_first_derivative_neg")) {
          rcl_interfaces::msg::ParameterDescriptor descriptor;
          descriptor.description = "Maximum value of the first derivative if **value** is negative, e.g. [m/s^2]";
          descriptor.read_only = false;
          auto parameter = to_parameter_value(updated_params.max_first_derivative_neg);
          parameters_interface_->declare_parameter(prefix_ + "max_first_derivative_neg", parameter, descriptor);
      }
      if (!parameters_interface_->has_parameter(prefix_ + "min_first_derivative_neg")) {
          rcl_interfaces::msg::ParameterDescriptor descriptor;
          descriptor.description = "Minimum value of the first derivative if **value** is negative, e.g. [m/s^2]";
          descriptor.read_only = false;
          auto parameter = to_parameter_value(updated_params.min_first_derivative_neg);
          parameters_interface_->declare_parameter(prefix_ + "min_first_derivative_neg", parameter, descriptor);
      }
      if (!parameters_interface_->has_parameter(prefix_ + "max_second_derivative")) {
          rcl_interfaces::msg::ParameterDescriptor descriptor;
          descriptor.description = "Maximum value of the second derivative, e.g. [m/s^3]";
          descriptor.read_only = false;
          auto parameter = to_parameter_value(updated_params.max_second_derivative);
          parameters_interface_->declare_parameter(prefix_ + "max_second_derivative", parameter, descriptor);
      }
      if (!parameters_interface_->has_parameter(prefix_ + "min_second_derivative")) {
          rcl_interfaces::msg::ParameterDescriptor descriptor;
          descriptor.description = "Minimum value of the second derivative, e.g. [m/s^3]";
          descriptor.read_only = false;
          auto parameter = to_parameter_value(updated_params.min_second_derivative);
          parameters_interface_->declare_parameter(prefix_ + "min_second_derivative", parameter, descriptor);
      }
      // get parameters and fill struct fields
      rclcpp::Parameter param;
      param = parameters_interface_->get_parameter(prefix_ + "sampling_interval");
      RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
      if(auto validation_result = gt<double>(param, 0.0);
        !validation_result) {
          throw rclcpp::exceptions::InvalidParameterValueException(fmt::format("Invalid value set during initialization for parameter 'sampling_interval': {}", validation_result.error()));
      }
      updated_params.sampling_interval = param.as_double();
      param = parameters_interface_->get_parameter(prefix_ + "max_value");
      RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
      if(auto validation_result = control_filters::gt_eq_or_nan<double>(param, 0.0);
        !validation_result) {
          throw rclcpp::exceptions::InvalidParameterValueException(fmt::format("Invalid value set during initialization for parameter 'max_value': {}", validation_result.error()));
      }
      updated_params.max_value = param.as_double();
      param = parameters_interface_->get_parameter(prefix_ + "min_value");
      RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
      if(auto validation_result = control_filters::lt_eq_or_nan<double>(param, 0.0);
        !validation_result) {
          throw rclcpp::exceptions::InvalidParameterValueException(fmt::format("Invalid value set during initialization for parameter 'min_value': {}", validation_result.error()));
      }
      updated_params.min_value = param.as_double();
      param = parameters_interface_->get_parameter(prefix_ + "max_first_derivative_pos");
      RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
      if(auto validation_result = control_filters::gt_eq_or_nan<double>(param, 0.0);
        !validation_result) {
          throw rclcpp::exceptions::InvalidParameterValueException(fmt::format("Invalid value set during initialization for parameter 'max_first_derivative_pos': {}", validation_result.error()));
      }
      updated_params.max_first_derivative_pos = param.as_double();
      param = parameters_interface_->get_parameter(prefix_ + "min_first_derivative_pos");
      RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
      if(auto validation_result = control_filters::lt_eq_or_nan<double>(param, 0.0);
        !validation_result) {
          throw rclcpp::exceptions::InvalidParameterValueException(fmt::format("Invalid value set during initialization for parameter 'min_first_derivative_pos': {}", validation_result.error()));
      }
      updated_params.min_first_derivative_pos = param.as_double();
      param = parameters_interface_->get_parameter(prefix_ + "max_first_derivative_neg");
      RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
      if(auto validation_result = control_filters::gt_eq_or_nan<double>(param, 0.0);
        !validation_result) {
          throw rclcpp::exceptions::InvalidParameterValueException(fmt::format("Invalid value set during initialization for parameter 'max_first_derivative_neg': {}", validation_result.error()));
      }
      updated_params.max_first_derivative_neg = param.as_double();
      param = parameters_interface_->get_parameter(prefix_ + "min_first_derivative_neg");
      RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
      if(auto validation_result = control_filters::lt_eq_or_nan<double>(param, 0.0);
        !validation_result) {
          throw rclcpp::exceptions::InvalidParameterValueException(fmt::format("Invalid value set during initialization for parameter 'min_first_derivative_neg': {}", validation_result.error()));
      }
      updated_params.min_first_derivative_neg = param.as_double();
      param = parameters_interface_->get_parameter(prefix_ + "max_second_derivative");
      RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
      if(auto validation_result = control_filters::gt_eq_or_nan<double>(param, 0.0);
        !validation_result) {
          throw rclcpp::exceptions::InvalidParameterValueException(fmt::format("Invalid value set during initialization for parameter 'max_second_derivative': {}", validation_result.error()));
      }
      updated_params.max_second_derivative = param.as_double();
      param = parameters_interface_->get_parameter(prefix_ + "min_second_derivative");
      RCLCPP_DEBUG_STREAM(logger_, param.get_name() << ": " << param.get_type_name() << " = " << param.value_to_string());
      if(auto validation_result = control_filters::lt_eq_or_nan<double>(param, 0.0);
        !validation_result) {
          throw rclcpp::exceptions::InvalidParameterValueException(fmt::format("Invalid value set during initialization for parameter 'min_second_derivative': {}", validation_result.error()));
      }
      updated_params.min_second_derivative = param.as_double();


      updated_params.__stamp = clock_.now();
      update_internal_params(updated_params);
    }

    private:
      void update_internal_params(Params updated_params) {
        std::lock_guard<std::mutex> lock(mutex_);
        params_ = std::move(updated_params);
      }

      std::string prefix_;
      Params params_;
      rclcpp::Clock clock_;
      std::shared_ptr<rclcpp::node_interfaces::OnSetParametersCallbackHandle> handle_;
      std::shared_ptr<rclcpp::node_interfaces::NodeParametersInterface> parameters_interface_;

      // rclcpp::Logger cannot be default-constructed
      // so we must provide a initialization here even though
      // every one of our constructors initializes logger_
      rclcpp::Logger logger_ = rclcpp::get_logger("rate_limiter");
      std::mutex mutable mutex_;
  };

} // namespace rate_limiter
