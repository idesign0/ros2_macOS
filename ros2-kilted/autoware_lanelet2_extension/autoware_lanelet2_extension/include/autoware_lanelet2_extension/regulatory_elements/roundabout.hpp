// Copyright 2023 Tier IV, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__ROUNDABOUT_HPP_
#define AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__ROUNDABOUT_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_lanelet2_extension/regulatory_elements/Forward.hpp>

#include <lanelet2_core/primitives/Lanelet.h>

#include <memory>
#include <unordered_set>

namespace lanelet::autoware
{

inline namespace format_v2
{
class Roundabout : public lanelet::RegulatoryElement
{
public:
  using Ptr = std::shared_ptr<Roundabout>;
  static constexpr char RuleName[] = "roundabout";

  struct AutowareRoleNameString
  {
    static constexpr const char Entry[] = "entry";
    static constexpr const char Exit[] = "exit";
    static constexpr const char Internal[] = "internal";
  };

  static Ptr make(
    Id id, const AttributeMap & attributes, const lanelet::Lanelets & roundabout_entry_lanelets,
    const lanelet::Lanelets & roundabout_exit_lanelets,
    const lanelet::Lanelets & roundabout_internal_lanelets)
  {
    return Ptr{new Roundabout(
      id, attributes, roundabout_entry_lanelets, roundabout_exit_lanelets,
      roundabout_internal_lanelets)};
  }

  /**
   * @brief get the relevant roundabout lanelet
   * @return lanelet
   */
  [[nodiscard]] lanelet::ConstLanelets roundaboutLanelets() const;
  /**
   * @brief get the relevant roundabout entry lanelet
   * @return lanelets
   */
  [[nodiscard]] lanelet::ConstLanelets roundaboutEntryLanelets() const;

  /**
   * @brief get the relevant roundabout exit lanelet
   * @return lanelets
   */
  [[nodiscard]] lanelet::ConstLanelets roundaboutExitLanelets() const;
  /**
   * @brief get the relevant roundabout internal lanelet
   * @return lanelets
   */
  [[nodiscard]] lanelet::ConstLanelets roundaboutInternalLanelets() const;

  /**
   * @brief Check if the given lanelet is an entry lanelet
   * @return true if the lanelet is an entry lanelet, false otherwise
   */
  [[nodiscard]] bool isEntryLanelet(const lanelet::Id & lanelet_id) const;

  /**
   * @brief Check if the given lanelet is an internal lanelet
   * @return true if the lanelet is an internal lanelet, false otherwise
   */
  [[nodiscard]] bool isInternalLanelet(const lanelet::Id & lanelet_id) const;

  /**
   * @brief Check if the given lanelet is an exit lanelet
   * @return true if the lanelet is an exit lanelet, false otherwise
   */
  [[nodiscard]] bool isExitLanelet(const lanelet::Id & lanelet_id) const;

  /**
   * @brief Check if the given lanelet is a roundabout lanelet
   * @return true if the lanelet is a roundabout lanelet, false otherwise
   */
  [[nodiscard]] bool isRoundaboutLanelet(const lanelet::Id & lanelet_id) const;

private:
  Roundabout(
    Id id, const AttributeMap & attributes, const lanelet::Lanelets & roundabout_entry_lanelets,
    const lanelet::Lanelets & roundabout_exit_lanelets,
    const lanelet::Lanelets & roundabout_internal_lanelets);

  void cacheLaneletIds();

  std::unordered_set<lanelet::Id> entry_lanelet_ids_;
  std::unordered_set<lanelet::Id> exit_lanelet_ids_;
  std::unordered_set<lanelet::Id> internal_lanelet_ids_;

  // the following lines are required so that lanelet2 can create this object
  // when loading a map with this regulatory element
  friend class RegisterRegulatoryElement<Roundabout>;
  explicit Roundabout(const lanelet::RegulatoryElementDataPtr & data);
};
}  // namespace format_v2

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__ROUNDABOUT_HPP_
