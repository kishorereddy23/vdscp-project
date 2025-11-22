#ifndef VDSPROJECT_TESTS_H
#define VDSPROJECT_TESTS_H

#include <gtest/gtest.h>
#include "Manager.h"

using namespace ClassProject;

// Test Fixture
class ManagerTest : public ::testing::Test {
protected:
    Manager manager;
    BDD_ID a, b, c, d;
    
    void SetUp() override {
        a = manager.createVar("a");
        b = manager.createVar("b");
        c = manager.createVar("c");
        d = manager.createVar("d");
    }
};

// Basic Tests
TEST(ManagerBasicTest, TrueAndFalse) {
    Manager mgr;
    EXPECT_EQ(mgr.False(), 0);
    EXPECT_EQ(mgr.True(), 1);
}

TEST(ManagerBasicTest, InitialTableSize) {
    Manager mgr;
    EXPECT_EQ(mgr.uniqueTableSize(), 2);
}

TEST(ManagerBasicTest, CreateVariable) {
    Manager mgr;
    BDD_ID a = mgr.createVar("a");
    BDD_ID b = mgr.createVar("b");
    EXPECT_EQ(a, 2);
    EXPECT_EQ(b, 3);
}

TEST(ManagerBasicTest, IsConstant) {
    Manager mgr;
    EXPECT_TRUE(mgr.isConstant(mgr.True()));
    EXPECT_TRUE(mgr.isConstant(mgr.False()));
    BDD_ID a = mgr.createVar("a");
    EXPECT_FALSE(mgr.isConstant(a));
}

TEST(ManagerBasicTest, IsVariable) {
    Manager mgr;
    BDD_ID a = mgr.createVar("a");
    EXPECT_TRUE(mgr.isVariable(a));
    EXPECT_FALSE(mgr.isVariable(mgr.True()));
}

TEST_F(ManagerTest, TopVar) {
    EXPECT_EQ(manager.topVar(a), a);
    EXPECT_EQ(manager.topVar(b), b);
}

TEST_F(ManagerTest, GetTopVarName) {
    EXPECT_EQ(manager.getTopVarName(a), "a");
    EXPECT_EQ(manager.getTopVarName(b), "b");
}

TEST_F(ManagerTest, NegTrue) {
    EXPECT_EQ(manager.neg(manager.True()), manager.False());
}

TEST_F(ManagerTest, NegFalse) {
    EXPECT_EQ(manager.neg(manager.False()), manager.True());
}

TEST_F(ManagerTest, DoubleNeg) {
    EXPECT_EQ(manager.neg(manager.neg(a)), a);
}

TEST_F(ManagerTest, AndWithFalse) {
    EXPECT_EQ(manager.and2(a, manager.False()), manager.False());
}

TEST_F(ManagerTest, AndWithTrue) {
    EXPECT_EQ(manager.and2(a, manager.True()), a);
}

TEST_F(ManagerTest, OrWithFalse) {
    EXPECT_EQ(manager.or2(a, manager.False()), a);
}

TEST_F(ManagerTest, OrWithTrue) {
    EXPECT_EQ(manager.or2(a, manager.True()), manager.True());
}

TEST_F(ManagerTest, XorWithSelf) {
    EXPECT_EQ(manager.xor2(a, a), manager.False());
}

TEST_F(ManagerTest, XnorWithSelf) {
    EXPECT_EQ(manager.xnor2(a, a), manager.True());
}

TEST_F(ManagerTest, CoFactorTrue) {
    EXPECT_EQ(manager.coFactorTrue(a, a), manager.True());
    EXPECT_EQ(manager.coFactorFalse(a, a), manager.False());
}

TEST_F(ManagerTest, IteBasic) {
    EXPECT_EQ(manager.ite(manager.True(), a, b), a);
    EXPECT_EQ(manager.ite(manager.False(), a, b), b);
}

TEST_F(ManagerTest, FindNodes) {
    std::set<BDD_ID> nodes;
    manager.findNodes(a, nodes);
    EXPECT_EQ(nodes.size(), 3);
}

TEST_F(ManagerTest, FindVars) {
    std::set<BDD_ID> vars;
    manager.findVars(a, vars);
    EXPECT_EQ(vars.size(), 1);
}

TEST_F(ManagerTest, DeMorgan) {
    BDD_ID lhs = manager.neg(manager.and2(a, b));
    BDD_ID rhs = manager.or2(manager.neg(a), manager.neg(b));
    EXPECT_EQ(lhs, rhs);
}

#endif
