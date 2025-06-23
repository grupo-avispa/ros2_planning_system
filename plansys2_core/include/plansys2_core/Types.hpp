// Copyright 2019 Intelligent Robotics Lab
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

#ifndef PLANSYS2_CORE__TYPES_HPP_
#define PLANSYS2_CORE__TYPES_HPP_

#include <string>
#include <vector>

#include "plansys2_msgs/msg/node.hpp"
#include "plansys2_msgs/msg/param.hpp"
#include "plansys2_msgs/msg/tree.hpp"

#include "plansys2_pddl_parser/Utils.hpp"

namespace plansys2
{

/**
 * @class plansys2::Instance
 * @brief Represents an instance (object) in the planning domain.
 *
 * Inherits from plansys2_msgs::msg::Param and provides constructors for easy creation
 * from strings or existing Param messages.
 */
class Instance : public plansys2_msgs::msg::Param
{
public:
  /**
   * @brief Default constructor.
   */
  Instance()
  : plansys2_msgs::msg::Param() {}

  /**
   * @brief Construct an Instance from a name and optional type.
   * @param[in] name The name of the instance.
   * @param[in] type The type of the instance (optional).
   */
  explicit Instance(const std::string & name, const std::string & type = {})
  : plansys2_msgs::msg::Param(parser::pddl::fromStringParam(name, type)) {}

  /**
   * @brief Construct an Instance from an existing Param message.
   * @param[in] instance The Param message.
   */
  Instance(const plansys2_msgs::msg::Param & instance)  // NOLINT(runtime/explicit)
  : plansys2_msgs::msg::Param(instance) {}
};

/**
 * @class plansys2::Predicate
 * @brief Represents a predicate in the planning domain.
 *
 * Inherits from plansys2_msgs::msg::Node and provides constructors for easy creation
 * from strings or existing Node messages.
 */
class Predicate : public plansys2_msgs::msg::Node
{
public:
  /**
   * @brief Default constructor.
   */
  Predicate()
  : plansys2_msgs::msg::Node() {}

  /**
   * @brief Construct a Predicate from a string representation.
   * @param[in] pred The predicate as a string.
   */
  explicit Predicate(const std::string & pred)
  : plansys2_msgs::msg::Node(parser::pddl::fromStringPredicate(pred)) {}

  /**
   * @brief Construct a Predicate from an existing Node message.
   * @param[in] pred The Node message.
   */
  Predicate(const plansys2_msgs::msg::Node & pred)  // NOLINT(runtime/explicit)
  : plansys2_msgs::msg::Node(pred) {}
};

/**
 * @class plansys2::Function
 * @brief Represents a function in the planning domain.
 *
 * Inherits from plansys2_msgs::msg::Node and provides constructors for easy creation
 * from strings or existing Node messages.
 */
class Function : public plansys2_msgs::msg::Node
{
public:
  /**
   * @brief Default constructor.
   */
  Function()
  : plansys2_msgs::msg::Node() {}

  /**
   * @brief Construct a Function from a string representation.
   * @param[in] func The function as a string.
   */
  explicit Function(const std::string & func)
  : plansys2_msgs::msg::Node(parser::pddl::fromStringFunction(func)) {}

  /**
   * @brief Construct a Function from an existing Node message.
   * @param[in] func The Node message.
   */
  Function(const plansys2_msgs::msg::Node & func)  // NOLINT(runtime/explicit)
  : plansys2_msgs::msg::Node(func) {}
};

/**
 * @class plansys2::Goal
 * @brief Represents a goal in the planning domain.
 *
 * Inherits from plansys2_msgs::msg::Tree and provides constructors for easy creation
 * from strings or existing Tree messages.
 */
class Goal : public plansys2_msgs::msg::Tree
{
public:
  /**
   * @brief Default constructor.
   */
  Goal()
  : plansys2_msgs::msg::Tree() {}

  /**
   * @brief Construct a Goal from a string representation.
   * @param[in] goal The goal as a string.
   */
  explicit Goal(const std::string & goal)
  : plansys2_msgs::msg::Tree(parser::pddl::fromString(goal)) {}

  /**
   * @brief Construct a Goal from an existing Tree message.
   * @param[in] goal The Tree message.
   */
  Goal(const plansys2_msgs::msg::Tree & goal)  // NOLINT(runtime/explicit)
  : plansys2_msgs::msg::Tree(goal) {}
};

/**
 * @brief Utility function to convert a vector of one type to another.
 * @tparam toT The target type.
 * @tparam fromT The source type.
 * @param[in] in_vector The input vector to convert.
 * @return std::vector<toT> The converted vector.
 */
template<class toT, class fromT>
std::vector<toT>
convertVector(const std::vector<fromT> & in_vector)
{
  std::vector<toT> ret;
  for (const auto & item : in_vector) {
    ret.push_back(item);
  }
  return ret;
}

}  // namespace plansys2

#endif  // PLANSYS2_CORE__TYPES_HPP_
