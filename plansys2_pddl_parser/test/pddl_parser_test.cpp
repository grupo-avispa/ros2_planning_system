// Copyright 2022 Marco Roveri - University of Trento
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

#include <algorithm>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "ament_index_cpp/get_package_share_directory.hpp"
#include "gtest/gtest.h"
#include "plansys2_pddl_parser/Instance.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class PDDLParserTestCase : public ::testing::Test
{
protected:
  static void SetUpTestCase() {rclcpp::init(0, nullptr);}
};

TEST(PDDLParserTestCase, pddl_parser)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_pddl_parser");
  std::string domain_file = pkgpath + "/pddl/dom1.pddl";
  std::string instance_file = pkgpath + "/pddl/prob1.pddl";

  std::ifstream domain_ifs(domain_file);
  ASSERT_TRUE(domain_ifs.good());
  std::string domain_str(
    (std::istreambuf_iterator<char>(domain_ifs)), std::istreambuf_iterator<char>());
  ASSERT_NE(domain_str, "");
  std::ifstream instance_ifs(instance_file);
  ASSERT_TRUE(instance_ifs.good());
  std::string instance_str(
    (std::istreambuf_iterator<char>(instance_ifs)), std::istreambuf_iterator<char>());

  ASSERT_NE(instance_str, "");
  // Read domain and instance
  bool okparse = false;
  bool okprint = false;
  try {
    parser::pddl::Domain domain(domain_str);
    parser::pddl::Instance instance(domain, instance_str);
    okparse = true;
    try {
      std::cout << domain << std::endl;
      std::cout << instance << std::endl;
      okprint = true;
    } catch (std::runtime_error e) {
      std::cerr << e.what() << std::endl;
    }
  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl;
  }
  ASSERT_TRUE(okparse);
  ASSERT_TRUE(okprint);
}

TEST(PDDLParserTestCase, exists_get_tree)
{
  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_pddl_parser");
  std::string domain_file = pkgpath + "/pddl/dom1.pddl";

  std::ifstream domain_ifs(domain_file);
  std::string domain_str(
    (std::istreambuf_iterator<char>(domain_ifs)), std::istreambuf_iterator<char>());
  parser::pddl::Domain domain(domain_str);

  auto action = domain.actions.get("action_test4");
  plansys2_msgs::msg::Tree tree;
  action->pre->getTree(tree, domain);
  std::string str = parser::pddl::toString(tree);

  ASSERT_EQ(
    str,
    "(and (exists (?1) (and (robot_at ?0 ?1)(charging_point_at ?1)))(and (>  (battery_level ?0) "
    "1.000000)(<  (battery_level ?0) 200.000000)))");

  plansys2_msgs::msg::Tree tree2;
  std::vector<std::string> replace = {"rob1"};
  action->pre->getTree(tree2, domain, replace);
  std::string str2 = parser::pddl::toString(tree2);
  ASSERT_EQ(
    str2,
    "(and (exists (?1) (and (robot_at rob1 ?1)(charging_point_at ?1)))(and (>  (battery_level "
    "rob1) 1.000000)(<  (battery_level rob1) 200.000000)))");

  auto action2 = domain.actions.get("action_test5");
  plansys2_msgs::msg::Tree tree3;
  action2->pre->getTree(tree3, domain);
  std::string str3 = parser::pddl::toString(tree3);
  ASSERT_EQ(str3, "(exists (?1 ?2) (and (robot_at ?0 ?1)(connected ?1 ?2)))");
}

TEST(PDDLParserTestCase, from_string_hyphen)
{
  auto predicate_hyphen = parser::pddl::fromStringPredicate("(predicate-hyphen ?x ?y)");
  auto expression_sub = parser::pddl::fromString("( - 3 4)");

  ASSERT_EQ(predicate_hyphen.node_type, plansys2_msgs::msg::Node::PREDICATE);
  ASSERT_EQ(predicate_hyphen.name, "predicate-hyphen");
  ASSERT_EQ(predicate_hyphen.parameters.size(), 2);
  ASSERT_EQ(predicate_hyphen.parameters[0].name, "?x");
  ASSERT_EQ(predicate_hyphen.parameters[1].name, "?y");
  
  ASSERT_EQ(expression_sub.nodes[0].node_type, plansys2_msgs::msg::Node::EXPRESSION);
  ASSERT_EQ(expression_sub.nodes[0].expression_type, plansys2_msgs::msg::Node::ARITH_SUB);
  ASSERT_EQ(expression_sub.nodes[0].children.size(), 2);
  ASSERT_EQ(expression_sub.nodes[1].node_type, plansys2_msgs::msg::Node::NUMBER);
  ASSERT_EQ(expression_sub.nodes[1].value, 3.0);
  ASSERT_EQ(expression_sub.nodes[2].node_type, plansys2_msgs::msg::Node::NUMBER);
  ASSERT_EQ(expression_sub.nodes[2].value, 4.0);
}

TEST(PDDLParserTestCase, test_remove_operators_before_parenthesis)
{
  std::string expr = "and (predicateA ?a)(predicateB ?b)";
  parser::pddl::removeOperatorBeforeParenthesis(expr);
  ASSERT_EQ(expr, "(predicateA ?a)(predicateB ?b)");

  expr = "or (predicateA ?a) (predicateB ?b)";
  parser::pddl::removeOperatorBeforeParenthesis(expr);
  ASSERT_EQ(expr, "(predicateA ?a) (predicateB ?b)");

  expr = "exists (?b) (and (predicateA ?a)(predicateB ?b))";
  parser::pddl::removeOperatorBeforeParenthesis(expr);
  ASSERT_EQ(expr, "(and (predicateA ?a)(predicateB ?b))");

  expr = "    exists (?b) (and (predicateA ?a)(predicateB ?b))";
  parser::pddl::removeOperatorBeforeParenthesis(expr);
  ASSERT_EQ(expr, "(and (predicateA ?a)(predicateB ?b))");

  expr = "= 3 5";
  parser::pddl::removeOperatorBeforeParenthesis(expr);
  ASSERT_EQ(expr, "3 5"); 

  expr = " = ?a a";
  parser::pddl::removeOperatorBeforeParenthesis(expr);
  ASSERT_EQ(expr, "?a a");
  
  expr = "+ ?a b";
  parser::pddl::removeOperatorBeforeParenthesis(expr);
  ASSERT_EQ(expr, "?a b");
  
  expr = "?s k";
  parser::pddl::removeOperatorBeforeParenthesis(expr);
  ASSERT_EQ(expr, "?s k");
}

TEST(PDDLParserTestCase, get_sub_expr)
{
  std::string expr = "(and (predicateA ?a) (exists (?b) (and (inferredA ?a)(inferredB ?b)(inferredAB ?a ?b)(not(=?a ?b)))))";
  std::vector<std::string> subexprs = parser::pddl::getSubExpr(expr);
  ASSERT_EQ(subexprs.size(), 2);
  ASSERT_EQ(subexprs[0], "(predicateA ?a)");
  ASSERT_EQ(subexprs[1], "(exists (?b) (and (inferredA ?a)(inferredB ?b)(inferredAB ?a ?b)(not(=?a ?b))))");

  subexprs = parser::pddl::getSubExpr(subexprs[1]);
  ASSERT_EQ(subexprs.size(), 1);
  ASSERT_EQ(subexprs[0], "(and (inferredA ?a)(inferredB ?b)(inferredAB ?a ?b)(not(=?a ?b)))");

  expr = "(= 3 5)";
  subexprs = parser::pddl::getSubExpr(expr);
  ASSERT_EQ(subexprs.size(), 2);
  ASSERT_EQ(subexprs[0], "3");
  ASSERT_EQ(subexprs[1], "5");
  
  expr = "(= ?a a)";
  subexprs = parser::pddl::getSubExpr(expr);
  ASSERT_EQ(subexprs.size(), 2);
  ASSERT_EQ(subexprs[0], "?a");
  ASSERT_EQ(subexprs[1], "a");

  expr = "(+ ?a b)";
  subexprs = parser::pddl::getSubExpr(expr);
  ASSERT_EQ(subexprs.size(), 2);
  ASSERT_EQ(subexprs[0], "?a");
  ASSERT_EQ(subexprs[1], "b");

  expr = "(?s k)";
  subexprs = parser::pddl::getSubExpr(expr);
  ASSERT_EQ(subexprs.size(), 2);
  ASSERT_EQ(subexprs[0], "?s");
  ASSERT_EQ(subexprs[1], "k");

  expr = "(- b ?sas)";
  subexprs = parser::pddl::getSubExpr(expr);
  ASSERT_EQ(subexprs.size(), 2);
  ASSERT_EQ(subexprs[0], "b");
  ASSERT_EQ(subexprs[1], "?sas");

}

TEST(PDDLParserTestCase, get_node_type)
{
  ASSERT_EQ(parser::pddl::getNodeType("(and (predicateA ?a) (predicateB ?b))"), plansys2_msgs::msg::Node::AND);
  ASSERT_EQ(parser::pddl::getNodeType("(or (predicateA ?a) (predicateB ?b))"), plansys2_msgs::msg::Node::OR);
  ASSERT_EQ(parser::pddl::getNodeType("(not (predicateA ?a))"), plansys2_msgs::msg::Node::NOT);
  ASSERT_EQ(parser::pddl::getNodeType("(exists (?b) (and (predicateA ?a)(predicateB ?b)))"), plansys2_msgs::msg::Node::EXISTS);
  ASSERT_EQ(parser::pddl::getNodeType("(= 3 5)"), plansys2_msgs::msg::Node::EXPRESSION);
  ASSERT_EQ(parser::pddl::getNodeType("(+ ?a b)"), plansys2_msgs::msg::Node::EXPRESSION);
  ASSERT_EQ(parser::pddl::getNodeType("(- b ?sas)"), plansys2_msgs::msg::Node::EXPRESSION);
  ASSERT_EQ(parser::pddl::getNodeType("?a"), plansys2_msgs::msg::Node::PARAMETER);
  ASSERT_EQ(parser::pddl::getNodeType("3"), plansys2_msgs::msg::Node::NUMBER);
}