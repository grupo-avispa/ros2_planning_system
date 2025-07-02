install(DIRECTORY pddl
  DESTINATION share/${PROJECT_NAME}/test_2
)

ament_add_gtest(test_2 test_2.cpp)
target_link_libraries(test_2
  ${PROJECT_NAME}
  plansys2_domain_expert::plansys2_domain_expert
  plansys2_problem_expert::plansys2_problem_expert
  plansys2_planner::plansys2_planner
)
