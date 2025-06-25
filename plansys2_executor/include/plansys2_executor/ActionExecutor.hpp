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

#ifndef PLANSYS2_EXECUTOR__ACTIONEXECUTOR_HPP_
#define PLANSYS2_EXECUTOR__ACTIONEXECUTOR_HPP_

#include <string>
#include <memory>
#include <vector>

#include "plansys2_msgs/msg/action.hpp"
#include "plansys2_msgs/msg/action_execution.hpp"
#include "plansys2_msgs/msg/action_execution_info.hpp"
#include "plansys2_msgs/msg/durative_action.hpp"
#include "plansys2_msgs/msg/param.hpp"
#include "plansys2_msgs/msg/plan_item.hpp"
#include "plansys2_pddl_parser/Utils.hpp"
#include "behaviortree_cpp/behavior_tree.h"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"

namespace plansys2
{

/**
 * @class plansys2::ActionExecutor
 * @brief Class that manages the execution of a PDDL action by communicating with action performers.
 *
 * This class handles the lifecycle of action execution by sending requests to appropriate
 * action performers, monitoring their progress, and reporting completion status.
 * It maintains state information about the execution and communicates over the actions_hub topic.
 */
class ActionExecutor
{
public:
  /**
   * @brief Status enumeration representing the action execution state.
   */
  enum Status
  {
    IDLE,
    DEALING,
    RUNNING,
    SUCCESS,
    FAILURE,
    CANCELLED
  };

  using Ptr = std::shared_ptr<ActionExecutor>;

  /**
   * @brief Factory method to create a shared pointer to an ActionExecutor.
   *
   * @param[in] action The action expression to execute (in PDDL format).
   * @param[in] node The lifecycle node used for ROS communications.
   * @return Shared pointer to the newly created ActionExecutor.
   */
  static Ptr make_shared(
    const std::string & action,
    rclcpp_lifecycle::LifecycleNode::SharedPtr node)
  {
    return std::make_shared<ActionExecutor>(action, node);
  }

  /**
   * @brief Constructor for the ActionExecutor.
   *
   * @param[in] action The action expression to execute (in PDDL format).
   * @param[in] node The lifecycle node used for ROS communications.
   */
  explicit ActionExecutor(
    const std::string & action, rclcpp_lifecycle::LifecycleNode::SharedPtr node);

  /**
   * @brief Destructor for the ActionExecutor.
   */
  ~ActionExecutor();

  /**
   * @brief Process one execution cycle for the action.
   *
   * Advances the state machine based on the current state:
   * - IDLE → DEALING: sends a request for performers
   * - DEALING: checks for timeout
   * - RUNNING: allows feedback to be processed
   *
   * @param[in] now Current ROS time.
   * @return BT::NodeStatus BehaviorTree status corresponding to the current execution state.
   */
  BT::NodeStatus tick(const rclcpp::Time & now);

  /**
   * @brief Cancel the action execution.
   *
   * Sets the state to CANCELLED and sends a cancellation message to the performer.
   */
  void cancel();

  /**
   * @brief Get the current status as a BehaviorTree node status.
   *
   * @return BT::NodeStatus The current status as a BehaviorTree node status.
   */
  BT::NodeStatus get_status();

  /**
   * @brief Check if the action execution has finished.
   *
   * @return true if the action has succeeded or failed, false otherwise.
   */
  bool is_finished();

  // Methods for debug

  /**
   * @brief Get the internal execution status.
   *
   * @return Status The current status enum value.
   */
  Status get_internal_status() const {return state_;}

  /**
   * @brief Set the internal execution status.
   *
   * @param[in] state The new status to set.
   */
  void set_internal_status(Status state) {state_ = state;}

  /**
   * @brief Get the name of the action being executed.
   *
   * @return std::string The action name.
   */
  std::string get_action_name() const {return action_name_;}

  /**
   * @brief Get the parameters of the action being executed.
   *
   * @return std::vector<std::string> Vector of parameter strings.
   */
  std::vector<std::string> get_action_params() const {return action_params_;}

  /**
   * @brief Get the time when the action execution started.
   *
   * @return rclcpp::Time The start time of the execution.
   */
  rclcpp::Time get_start_time() const {return start_execution_;}

  /**
   * @brief Get the current ROS time from the node.
   *
   * @return rclcpp::Time Current ROS time.
   */
  rclcpp::Time get_current_time() const {return node_->now();}

  /**
   * @brief Get the time of the last state change.
   *
   * @return ROS time of the last state change.
   */
  rclcpp::Time get_status_time() const {return state_time_;}

  /**
   * @brief Get the current feedback from the action performer.
   *
   * @return std::string containing the feedback message.
   */
  std::string get_feedback() const {return feedback_;}

  /**
   * @brief Get the completion percentage of the action.
   *
   * @return float Completion percentage (0.0 to 1.0).
   */
  float get_completion() const {return completion_;}

  /**
   * @brief Clean up resources used by the executor.
   */
  void clean_up();

  plansys2_msgs::msg::ActionExecution::SharedPtr last_msg_;

protected:
  /**
   * @brief Process messages from the actions hub.
   *
   * Handles different message types:
   * - RESPONSE: From performers accepting the request
   * - FEEDBACK: Updates on execution progress
   * - FINISH: Final completion status
   *
   * @param[in] msg The action execution message received.
   */
  void action_hub_callback(plansys2_msgs::msg::ActionExecution::SharedPtr msg);

  /**
   * @brief Send a request for performers to execute this action.
   *
   * Publishes a REQUEST message to the actions_hub topic.
   */
  void request_for_performers();

  /**
   * @brief Confirm a performer for this action.
   *
   * Sends a CONFIRM message to the selected performer.
   *
   * @param[in] node_id ID of the performer to confirm.
   */
  void confirm_performer(const std::string & node_id);

  /**
   * @brief Reject a performer for this action.
   *
   * Sends a REJECT message to the performer.
   *
   * @param[in] node_id ID of the performer to reject.
   */
  void reject_performer(const std::string & node_id);

  /**
   * @brief Extract the action name from an action expression.
   *
   * Parses a PDDL action expression to extract just the action name.
   *
   * @param[in] action_expr The action expression to parse.
   * @return std::string The extracted action name.
   */
  std::string get_name(const std::string & action_expr);

  /**
   * @brief Extract parameters from an action expression.
   *
   * Parses a PDDL action expression to extract the parameter values.
   *
   * @param[in] action_expr The action expression to parse.
   * @return std::vector<std::string> Vector of extracted parameter strings.
   */
  std::vector<std::string> get_params(const std::string & action_expr);

  /**
   * @brief Timeout handler for performer requests.
   *
   * Called when no performer responds in time, logs a warning and retries.
   */
  void wait_timeout();

  rclcpp_lifecycle::LifecycleNode::SharedPtr node_;

  Status state_;
  rclcpp::Time state_time_;
  rclcpp::Time start_execution_;

  std::string action_;
  std::string action_name_;
  std::string current_performer_id_;
  std::vector<std::string> action_params_;

  std::string feedback_;
  float completion_;

  rclcpp_lifecycle::LifecyclePublisher<plansys2_msgs::msg::ActionExecution>::SharedPtr
    action_hub_pub_;
  rclcpp::Subscription<plansys2_msgs::msg::ActionExecution>::SharedPtr action_hub_sub_;

  rclcpp::TimerBase::SharedPtr waiting_timer_;
};

/**
 * @brief Structure that holds either an Action or DurativeAction message.
 *
 * This struct provides a type-safe container for different action types and
 * offers a unified interface to access their properties regardless of type.
 */
struct ActionVariant
{
  using shared_ptr_action = std::shared_ptr<plansys2_msgs::msg::Action>;
  using shared_ptr_durative = std::shared_ptr<plansys2_msgs::msg::DurativeAction>;

  /**
   * @brief Variant that holds either an Action or DurativeAction.
   */
  std::variant<
    std::shared_ptr<plansys2_msgs::msg::Action>,
    std::shared_ptr<plansys2_msgs::msg::DurativeAction>> action;

  /**
   * @brief Assignment operator for Action messages.
   *
   * @param[in] ptr Shared pointer to an Action message.
   * @return Reference to this ActionVariant.
   */
  ActionVariant & operator=(shared_ptr_action ptr)
  {
    action = ptr;
    return *this;
  }

  /**
   * @brief Assignment operator for DurativeAction messages.
   *
   * @param[in] ptr Shared pointer to a DurativeAction message.
   * @return Reference to this ActionVariant.
   */
  ActionVariant & operator=(shared_ptr_durative ptr)
  {
    action = ptr;
    return *this;
  }

  /**
   * @brief Get the string representation of the action.
   *
   * @return String representation of the action with its parameters.
   */
  std::string get_action_string() const
  {
    std::string action_string;
    if (std::holds_alternative<shared_ptr_action>(action)) {
      action_string = parser::pddl::nameActionsToString(
        std::get<shared_ptr_action>(action));
    } else if (std::holds_alternative<shared_ptr_durative>(action)) {
      action_string = parser::pddl::nameActionsToString(
        std::get<shared_ptr_durative>(action));
    }
    return action_string;
  }

  /**
   * @brief Get the name of the action.
   *
   * @return std::string  The action name without parameters.
   */
  std::string get_action_name() const
  {
    std::string action_name;
    if (std::holds_alternative<shared_ptr_action>(action)) {
      action_name = std::get<shared_ptr_action>(action)->name;
    } else if (std::holds_alternative<shared_ptr_durative>(action)) {
      action_name = std::get<shared_ptr_durative>(action)->name;
    }
    return action_name;
  }

  /**
   * @brief Get the parameters of the action.
   *
   * @return std::vector<plansys2_msgs::msg::Param> Vector of parameter objects.
   */
  std::vector<plansys2_msgs::msg::Param> get_action_params() const
  {
    std::vector<plansys2_msgs::msg::Param> params;
    if (std::holds_alternative<shared_ptr_action>(action)) {
      params = std::get<shared_ptr_action>(action)->parameters;
    } else if (std::holds_alternative<shared_ptr_durative>(action)) {
      params = std::get<shared_ptr_durative>(action)->parameters;
    }
    return params;
  }

  /**
   * @brief Get the overall requirements for the action.
   *
   * For regular actions, these are the preconditions.
   * For durative actions, these are the over_all_requirements.
   *
   * @return plansys2_msgs::msg::Tree representing the requirements.
   */
  plansys2_msgs::msg::Tree get_overall_requirements() const
  {
    plansys2_msgs::msg::Tree reqs;
    if (std::holds_alternative<shared_ptr_action>(action)) {
      reqs = std::get<shared_ptr_action>(action)->preconditions;
    } else if (std::holds_alternative<shared_ptr_durative>(action)) {
      reqs = std::get<shared_ptr_durative>(action)->over_all_requirements;
    }
    return reqs;
  }

  /**
   * @brief Get the at-start requirements for the action.
   *
   * Only applicable for durative actions.
   *
   * @return plansys2_msgs::msg::Tree representing the at-start requirements.
   */
  plansys2_msgs::msg::Tree get_at_start_requirements() const
  {
    plansys2_msgs::msg::Tree reqs;
    if (std::holds_alternative<shared_ptr_durative>(action)) {
      reqs = std::get<shared_ptr_durative>(action)->at_start_requirements;
    }
    return reqs;
  }

  /**
   * @brief Get the at-end requirements for the action.
   *
   * Only applicable for durative actions.
   *
   * @return plansys2_msgs::msg::Tree representing the at-end requirements.
   */
  plansys2_msgs::msg::Tree get_at_end_requirements() const
  {
    plansys2_msgs::msg::Tree reqs;
    if (std::holds_alternative<shared_ptr_durative>(action)) {
      reqs = std::get<shared_ptr_durative>(action)->at_end_requirements;
    }
    return reqs;
  }

  /**
   * @brief Get the at-start effects of the action.
   *
   * Only applicable for durative actions.
   *
   * @return plansys2_msgs::msg::Tree representing the at-start effects.
   */
  plansys2_msgs::msg::Tree get_at_start_effects() const
  {
    plansys2_msgs::msg::Tree effects;
    if (std::holds_alternative<shared_ptr_durative>(action)) {
      effects = std::get<shared_ptr_durative>(action)->at_start_effects;
    }
    return effects;
  }

  /**
   * @brief Get the at-end effects of the action.
   *
   * For regular actions, these are the effects.
   * For durative actions, these are the at_end_effects.
   *
   * @return plansys2_msgs::msg::Tree representing the at-end effects.
   */
  plansys2_msgs::msg::Tree get_at_end_effects() const
  {
    plansys2_msgs::msg::Tree effects;
    if (std::holds_alternative<shared_ptr_action>(action)) {
      effects = std::get<shared_ptr_action>(action)->effects;
    } else if (std::holds_alternative<shared_ptr_durative>(action)) {
      effects = std::get<shared_ptr_durative>(action)->at_end_effects;
    }
    return effects;
  }

  /**
   * @brief Check if this is a regular (non-durative) action.
   *
   * @return true if this is a regular action, false otherwise.
   */
  bool is_action() const
  {
    return std::holds_alternative<shared_ptr_action>(action);
  }

  /**
   * @brief Check if this is a durative action.
   *
   * @return true if this is a durative action, false otherwise.
   */
  bool is_durative_action() const
  {
    return std::holds_alternative<shared_ptr_durative>(action);
  }
};

/**
 * @brief Structure that holds execution information for an action.
 *
 * This structure maintains the runtime state of an action's execution,
 * including timing information, effect application status, and error details.
 */
struct ActionExecutionInfo
{
  plansys2_msgs::msg::PlanItem plan_item;
  std::shared_ptr<ActionExecutor> action_executor = {nullptr};
  bool at_start_effects_applied = {false};
  bool at_end_effects_applied = {false};
  rclcpp::Time at_start_effects_applied_time;
  rclcpp::Time at_end_effects_applied_time;
  ActionVariant action_info;
  std::string execution_error_info;
  double duration;
  double duration_overrun_percentage = -1.0;
};

}  // namespace plansys2

#endif  // PLANSYS2_EXECUTOR__ACTIONEXECUTOR_HPP_
