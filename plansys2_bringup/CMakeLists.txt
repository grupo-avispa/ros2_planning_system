cmake_minimum_required(VERSION 3.5)

project(plansys2_bringup)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(plansys2_domain_expert REQUIRED)
find_package(plansys2_problem_expert REQUIRED)
find_package(plansys2_planner REQUIRED)
find_package(plansys2_executor REQUIRED)
find_package(plansys2_lifecycle_manager REQUIRED)

set(dependencies
    rclcpp::rclcpp
    plansys2_domain_expert::plansys2_domain_expert
    plansys2_problem_expert::plansys2_problem_expert
    plansys2_planner::plansys2_planner
    plansys2_executor::plansys2_executor
    plansys2_lifecycle_manager::plansys2_lifecycle_manager
)

set(package_dependencies
    rclcpp
    plansys2_domain_expert
    plansys2_problem_expert
    plansys2_planner
    plansys2_executor
    plansys2_lifecycle_manager
)


add_executable(plansys2_node
  src/plansys2_node.cpp
)
target_link_libraries(plansys2_node PUBLIC ${dependencies})
target_compile_definitions(plansys2_node PUBLIC "PLUGINLIB__DISABLE_BOOST_FUNCTIONS")

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

install(DIRECTORY launch params DESTINATION share/${PROJECT_NAME})
install(TARGETS
  plansys2_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_dependencies(${package_dependencies})
ament_package()
