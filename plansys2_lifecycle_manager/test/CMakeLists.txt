ament_add_gtest(lf_manager_test lf_manager_test.cpp)
target_link_libraries(lf_manager_test
  ${PROJECT_NAME}
  rclcpp_lifecycle::rclcpp_lifecycle
)
