# Copyright (c) 2025 Alberto J. Tudela Roldán
# Copyright (c) 2025 Grupo Avispa, DTE, Universidad de Málaga
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
PlanSys2 Executor Client.

This module provides specific classes for interacting with PlanSys2 Executor services.
"""

from typing import List, Optional
import rclpy
from rclpy.client import Client
from rclpy.node import Node

from plansys2_msgs.srv import GetOrderedSubGoals, GetPlan
from plansys2_msgs.msg import Plan, Tree


class ExecutorClient(Node):
    """
    Executor client for PlanSys2.

    This class provides convenient methods to interact with all Executor services
    in PlanSys2, wrapping the ROS2 service calls with proper error handling.
    """

    def __init__(self, node_name: str = 'executor_client', namespace: str = ''):
        """
        Initialize the Executor client.

        Parameters:
        node_name (str): Name of the ROS2 node.
        namespace (str): Namespace prefix for services.
        """
        super().__init__(node_name)

        # Setup namespace prefix
        self._namespace_prefix = f'/{namespace}' if namespace else ''

        self.get_logger().info(f'Executor Client "{node_name}" initialized')

    def _create_and_call_service(self, service_type, service_name: str, request):
        """
        Create a service client for the given service, call it and return the response.

        Parameters:
        service_type (Type): The ROS2 service type class.
        service_name (str): The name/topic of the service.
        request (Any): The service request object.

        Returns:
        Any: The service response or None if failed.
        """
        try:
            client: Client = self.create_client(service_type, service_name)

            if not client.wait_for_service(timeout_sec=5.0):
                self.get_logger().error(f'Service {service_name} not available')
                return None

            future = client.call_async(request)
            rclpy.spin_until_future_complete(self, future, timeout_sec=10.0)

            if future.done():
                return future.result()
            else:
                self.get_logger().error(f'Service call to {service_name} timed out')
                return None

        except Exception as e:
            self.get_logger().error(f'Error calling service {service_name}: {str(e)}')
            return None

    def get_ordered_sub_goals(self) -> Optional[List[Tree]]:
        """
        Get the ordered sub-goals from the executor.

        This method retrieves the list of sub-goals that the executor has identified
        for achieving the current problem goal, ordered by execution priority.

        Returns:
        Optional[List[Tree]]: List of sub-goals as Tree objects, or None if failed.
        """
        service_name = f'{self._namespace_prefix}/executor/get_ordered_sub_goals'
        request = GetOrderedSubGoals.Request()

        response = self._create_and_call_service(GetOrderedSubGoals, service_name, request)
        if response and response.success:
            self.get_logger().info('Successfully retrieved ordered sub-goals')
            return response.sub_goals
        else:
            error_msg = response.error_info if response else 'Service call failed'
            self.get_logger().error(f'Failed to retrieve ordered sub-goals: {error_msg}')
            return None

    def get_plan(self) -> Optional[Plan]:
        """
        Get the current plan from the executor.

        This method retrieves the complete plan that the executor is currently working on.

        Returns:
        Optional[Plan]: The current plan or None if failed.
        """
        service_name = f'{self._namespace_prefix}/executor/get_plan'
        request = GetPlan.Request()
        request.domain = ''
        request.problem = ''

        response = self._create_and_call_service(GetPlan, service_name, request)
        if response and response.success:
            self.get_logger().info('Successfully retrieved current plan')
            return response.plan
        else:
            error_msg = response.error_info if response else 'Service call failed'
            self.get_logger().error(f'Failed to retrieve current plan: {error_msg}')
            return None

    def get_remaining_plan(self) -> Optional[Plan]:
        """
        Get the remaining plan from the executor.

        This method retrieves the portion of the plan that has not yet been executed,
        which is useful for monitoring progress or handling plan modifications.

        Returns:
        Optional[Plan]: The remaining plan or None if failed.
        """
        service_name = f'{self._namespace_prefix}/executor/get_remaining_plan'
        request = GetPlan.Request()
        request.domain = ''
        request.problem = ''

        response = self._create_and_call_service(GetPlan, service_name, request)
        if response and response.success:
            self.get_logger().info('Successfully retrieved remaining plan')
            return response.plan
        else:
            error_msg = response.error_info if response else 'Service call failed'
            self.get_logger().error(f'Failed to retrieve remaining plan: {error_msg}')
            return None

    def print_executor_info(self) -> None:
        """
        Print comprehensive information about the executor state.

        Call multiple services to provide an overview of the executor's current status,
        including ordered sub-goals, current plan, and remaining plan.
        """
        self.get_logger().info('=== Executor Information ===')

        # Get ordered sub-goals
        sub_goals = self.get_ordered_sub_goals()
        if sub_goals:
            print(f'Ordered Sub-Goals ({len(sub_goals)}):')
            for i, goal in enumerate(sub_goals):
                print(f'  [{i+1}] {goal}')
        else:
            print('No sub-goals available')

        # Get current plan
        current_plan = self.get_plan()
        if current_plan:
            print(f'\nCurrent Plan ({len(current_plan.items)} items):')
            for i, item in enumerate(current_plan.items):
                action = item.action if hasattr(item, 'action') else 'N/A'
                duration = item.duration if hasattr(item, 'duration') else 0.0
                print(f'  [{i+1}] Action: {action}, Duration: {duration:.2f}s')
        else:
            print('\nNo current plan available')

        # Get remaining plan
        remaining_plan = self.get_remaining_plan()
        if remaining_plan:
            print(f'\nRemaining Plan ({len(remaining_plan.items)} items):')
            for i, item in enumerate(remaining_plan.items):
                action = item.action if hasattr(item, 'action') else 'N/A'
                duration = item.duration if hasattr(item, 'duration') else 0.0
                print(f'  [{i+1}] Action: {action}, Duration: {duration:.2f}s')

            # Calculate progress
            if current_plan and len(current_plan.items) > 0:
                total_items = len(current_plan.items)
                remaining_items = len(remaining_plan.items)
                completed_items = total_items - remaining_items
                progress = (completed_items / total_items) * 100
                print(f'\nExecution Progress: {completed_items}/{total_items} '
                      f'({progress:.1f}% complete)')
        else:
            print('\nNo remaining plan available')
            if current_plan and len(current_plan.items) > 0:
                print('Execution Progress: 100% (Plan completed)')

        print('=' * 40)
