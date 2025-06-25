ament_add_gtest(popf_test popf_test.cpp)
target_link_libraries(popf_test
  ${PROJECT_NAME}
  ament_index_cpp::ament_index_cpp
  dl
  pluginlib::pluginlib
)
target_compile_definitions(popf_test PUBLIC "PLUGINLIB__DISABLE_BOOST_FUNCTIONS")