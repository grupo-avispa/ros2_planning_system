// Copyright (c) 2024 Davide Faconti
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

#ifndef PLANSYS2_BT_ACTIONS__BTUTILS_HPP_
#define PLANSYS2_BT_ACTIONS__BTUTILS_HPP_

#include <string>

#include "behaviortree_cpp/behavior_tree.h"

namespace BT
{

/**
 * @brief Try reading an import port first, and if that doesn't work
 * fallback to reading directly the blackboard.
 * The blackboard must be passed explicitly because config() is private in BT.CPP 4.X
 *
 * @param bt_node node
 * @param blackboard the blackboard obtained with node->config().blackboard
 * @param param_name std::string
 * @param behavior_tree_node the node
 * @return <T>
 */
template<typename T> inline
bool getInputPortOrBlackboard(
  const BT::TreeNode & bt_node,
  const BT::Blackboard & blackboard,
  const std::string & param_name,
  T & value)
{
  if (bt_node.getInput<T>(param_name, value)) {
    return true;
  }
  if (blackboard.get<T>(param_name, value)) {
    return true;
  }
  return false;
}

// Macro to remove boiler plate when using getInputPortOrBlackboard
#define getInputOrBlackboard(name, value) \
  getInputPortOrBlackboard(*this, *(this->config().blackboard), name, value);

}  // namespace BT

#endif  // PLANSYS2_BT_ACTIONS__BTUTILS_HPP_
