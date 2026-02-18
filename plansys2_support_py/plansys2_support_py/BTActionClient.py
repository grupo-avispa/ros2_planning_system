# Copyright (c) 2025 Intelligent Robotics Lab
# Copyright (c) 2025 Grupo Avispa, DTE, Universidad de Málaga
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
PlanSys2 BT Action Client.

This module provides a client for sending expanded (JSON-serialized)
arguments to BTAction nodes via the 'add_expanded_arguments' ROS2 service.
"""

from typing import Dict, List, Union

from plansys2_msgs.msg import Argument
from plansys2_msgs.srv import AddExpandedArguments

import rclpy

from rclpy.client import Client
from rclpy.node import Node


class BTActionClient(Node):
    """
    BT Action client for PlanSys2.

    This class provides convenient methods to send complex (JSON-serialized)
    arguments to a BTAction node via the 'add_expanded_arguments' service,
    wrapping the ROS2 service calls with proper error handling.

    This allows passing data types beyond single words (e.g., phrases, dicts,
    lists) as BT blackboard entries.
    """

    def __init__(self, node_name: str = 'bt_action_client', namespace: str = '') -> None:
        """
        Initialize the BT Action client.

        Parameters
        ----------
        node_name : str, optional
            Name of the ROS2 node.
        namespace : str, optional
            Namespace prefix for services.

        """
        super().__init__(node_name=node_name, namespace=namespace)

        # Setup namespace prefix for building fully-qualified service names
        self._namespace_prefix = f'/{namespace}' if namespace else ''

        log_msg = f'BT Action Client "{node_name}" initialized'
        self.get_logger().debug(log_msg)

    def _create_and_call_service(self, service_type, service_name: str, request):
        """
        Create service client, call it and return response.

        Parameters
        ----------
        service_type : Type
            The ROS2 service type class.
        service_name : str
            The name/topic of the service.
        request : Any
            The service request object.

        Returns
        -------
        Any
            The service response or None if failed.

        """
        try:
            client: Client = self.create_client(service_type, service_name)

            if not client.wait_for_service(timeout_sec=5.0):
                error_msg = f'Service {service_name} not available'
                self.get_logger().error(error_msg)
                return None

            future = client.call_async(request)
            rclpy.spin_until_future_complete(self, future, timeout_sec=10.0)

            if future.done():
                return future.result()
            else:
                error_msg = f'Service call to {service_name} timed out'
                self.get_logger().error(error_msg)
                return None

        except RuntimeError as e:
            error_msg = f'Error calling service {service_name}: {str(e)}'
            self.get_logger().error(error_msg)
            return None

    def add_expanded_arguments(
        self,
        bt_action_node: str,
        arguments: Union[Dict[str, str], List[Argument]]
    ) -> bool:
        """
        Add expanded arguments to a BTAction node.

        Calls the 'add_expanded_arguments' service on the target BTAction node
        to inject arguments into its blackboard before activation.

        Parameters
        ----------
        bt_action_node : str
            Name of the target BTAction node hosting the service.
        arguments : Union[Dict[str, str], List[Argument]]
            Arguments to send. Can be:
            - Dict[str, str]: Dictionary with argument names and values
            - List[Argument]: List of Argument objects with name and value

        Returns
        -------
        bool
            True if the arguments were successfully sent and accepted.

        """
        service_name = (
            f'{self._namespace_prefix}/{bt_action_node}/add_expanded_arguments'
        )
        request = AddExpandedArguments.Request()

        # Convert arguments to list of Argument objects
        if isinstance(arguments, dict):
            request.arguments = [
                Argument(name=name, value=value)
                for name, value in arguments.items()
            ]
        else:
            # Assume List[Argument]
            request.arguments = arguments

        response = self._create_and_call_service(AddExpandedArguments, service_name, request)

        if response and response.success:
            self.get_logger().info(
                f'Successfully sent {len(request.arguments)} expanded arguments to {service_name}'
            )
            return True
        else:
            error_msg = (
                response.error_info if response else 'Service call failed'
            )
            self.get_logger().error(
                f'Failed to send expanded arguments to {service_name}: {error_msg}'
            )
            return False
