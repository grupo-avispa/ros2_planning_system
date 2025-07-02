ament_add_gtest(terminal_test terminal_test.cpp TIMEOUT 500)
target_link_libraries(terminal_test terminal readline ${dependencies})

install(DIRECTORY
  pddl
  DESTINATION share/${PROJECT_NAME}
)
