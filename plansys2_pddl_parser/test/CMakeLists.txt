ament_add_gtest(pddl_parser_test pddl_parser_test.cpp TIMEOUT 120)
target_link_libraries(pddl_parser_test ${dependencies} ${PROJECT_NAME})

install(DIRECTORY
  pddl
  DESTINATION share/${PROJECT_NAME}
)
