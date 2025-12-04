#ifndef VDSPROJECT_TESTS_H
#define VDSPROJECT_TESTS_H

#include <gtest/gtest.h>
#include "Manager.h"

using namespace ClassProject;

// Test Fixture: reusable manager with some variables
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

// ---------------- Basic Manager behavior ----------------

TEST(ManagerBasicTest, TrueAndFalse) {
    Manager mgr;
    EXPECT_EQ(mgr.False(), mgr.False());   // Just consistency
    EXPECT_EQ(mgr.True(),  mgr.True());
    EXPECT_NE(mgr.True(),  mgr.False());
}

TEST(ManagerBasicTest, InitialTableSize) {
    Manager mgr;
    EXPECT_EQ(mgr.uniqueTableSize(), 2);
}

TEST(ManagerBasicTest, CreateVariable) {
    Manager mgr;
    BDD_ID a = mgr.createVar("a");
    BDD_ID b = mgr.createVar("b");
    EXPECT_NE(a, b);
    EXPECT_EQ(mgr.uniqueTableSize(), 4);   // 0,1,a,b
}

TEST(ManagerBasicTest, IsConstantAndVariable) {
    Manager mgr;
    BDD_ID a = mgr.createVar("a");

    EXPECT_TRUE(mgr.isConstant(mgr.True()));
    EXPECT_TRUE(mgr.isConstant(mgr.False()));
    EXPECT_FALSE(mgr.isConstant(a));

    EXPECT_TRUE(mgr.isVariable(a));
    EXPECT_FALSE(mgr.isVariable(mgr.True()));
}

TEST(ManagerBasicTest, IsVariableBranchCoverage) {
    Manager mgr;
    BDD_ID a = mgr.createVar("a");

    // Branch where isConstant(x) == true
    EXPECT_FALSE(mgr.isVariable(mgr.True()));
    EXPECT_FALSE(mgr.isVariable(mgr.False()));

    // Branch where isConstant(x) == false
    EXPECT_TRUE(mgr.isVariable(a));
}

TEST_F(ManagerTest, And2CreatesAndReusesNode) {
    // First call: should create a node for a & b
    BDD_ID f1 = manager.and2(a, b);

    // Second call with same args: should return same BDD_ID
    BDD_ID f2 = manager.and2(a, b);
    EXPECT_EQ(f1, f2);

    // Trivial case: high == low path inside findOrCreateNode is hit
    // somewhere in ite when cofactors become equal.
}


// ---------------- Top variable handling ----------------

TEST_F(ManagerTest, TopVar) {
    EXPECT_EQ(manager.topVar(a), a);
    EXPECT_EQ(manager.topVar(b), b);
}

TEST_F(ManagerTest, GetTopVarName) {
    EXPECT_EQ(manager.getTopVarName(a), "a");
    EXPECT_EQ(manager.getTopVarName(b), "b");
}


TEST_F(ManagerTest, IteTopVarSelection) {
    // Assume IDs: a < b < c
    // Make i's top var not the smallest, so t or e can win.

    // Case 1: t has smaller top var than i
    BDD_ID res1 = manager.ite(b, a, manager.True());
    (void)res1; // avoid unused warning

    // Case 2: e has smaller top var than i
    BDD_ID res2 = manager.ite(b, manager.True(), a);
    (void)res2;
}

// ---------------- Negation ----------------

TEST_F(ManagerTest, NegTrue) {
    EXPECT_EQ(manager.neg(manager.True()), manager.False());
}

TEST_F(ManagerTest, NegFalse) {
    EXPECT_EQ(manager.neg(manager.False()), manager.True());
}

TEST_F(ManagerTest, DoubleNeg) {
    EXPECT_EQ(manager.neg(manager.neg(a)), a);
}

// ---------------- Binary logic ops: and/or/xor/xnor ----------------

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

// Extra: nand2/nor2/xnor2 relations

TEST_F(ManagerTest, NandIsNegAnd) {
    BDD_ID and_ab  = manager.and2(a, b);
    BDD_ID nand_ab = manager.nand2(a, b);
    EXPECT_EQ(nand_ab, manager.neg(and_ab));
}

TEST_F(ManagerTest, NorIsNegOr) {
    BDD_ID or_ab  = manager.or2(a, b);
    BDD_ID nor_ab = manager.nor2(a, b);
    EXPECT_EQ(nor_ab, manager.neg(or_ab));
}

TEST_F(ManagerTest, XnorIsNegXor) {
    BDD_ID xor_ab  = manager.xor2(a, b);
    BDD_ID xnor_ab = manager.xnor2(a, b);
    EXPECT_EQ(xnor_ab, manager.neg(xor_ab));
}

// ---------------- Cofactors ----------------

TEST_F(ManagerTest, CoFactorTrueFalseOnVariable) {
    EXPECT_EQ(manager.coFactorTrue(a, a), manager.True());
    EXPECT_EQ(manager.coFactorFalse(a, a), manager.False());
}

// New: cofactors of a composed function and one-argument versions
TEST_F(ManagerTest, CoFactorTrueFalseOnAnd) {
    BDD_ID f = manager.and2(a, b);

    // cofactor w.r.t. a
    EXPECT_EQ(manager.coFactorTrue(f, a),  manager.and2(manager.True(),  b));
    EXPECT_EQ(manager.coFactorFalse(f, a), manager.and2(manager.False(), b));

    // one-argument cofactors just follow high/low of that node
    BDD_ID f_true  = manager.coFactorTrue(f);
    BDD_ID f_false = manager.coFactorFalse(f);
    // we don't know exact IDs, but they must be some nodes reachable from f
    std::set<BDD_ID> nodes;
    manager.findNodes(f, nodes);
    EXPECT_TRUE(nodes.count(f_true));
    EXPECT_TRUE(nodes.count(f_false));
}

// ---------------- ITE behavior ----------------

TEST_F(ManagerTest, IteBasic) {
    EXPECT_EQ(manager.ite(manager.True(),  a, b), a);
    EXPECT_EQ(manager.ite(manager.False(), a, b), b);
}

// New: ITE with non-constant condition
TEST_F(ManagerTest, IteActsLikeVariable) {
    BDD_ID res = manager.ite(a, manager.True(), manager.False());
    // behaves like variable a
    EXPECT_EQ(res, a);
}

// ---------------- Structure: nodes and vars ----------------

TEST_F(ManagerTest, FindNodesSingleVar) {
    std::set<BDD_ID> nodes;
    manager.findNodes(a, nodes);
    // a plus 0 and 1 terminals
    EXPECT_EQ(nodes.size(), 3);
}

TEST_F(ManagerTest, FindVarsSingleVar) {
    std::set<BDD_ID> vars;
    manager.findVars(a, vars);
    EXPECT_EQ(vars.size(), 1);
}

// New: nodes and vars for a composed function
TEST_F(ManagerTest, FindNodesAndVarsOnAnd) {
    BDD_ID f = manager.and2(a, b);

    std::set<BDD_ID> nodes;
    manager.findNodes(f, nodes);
    EXPECT_GE(nodes.size(), 4u); // at least a,b,0,1

    std::set<BDD_ID> vars;
    manager.findVars(f, vars);
    EXPECT_EQ(vars.size(), 2);
}

// ---------------- Equivalence: DeMorgan ----------------

TEST_F(ManagerTest, DeMorgan) {
    BDD_ID lhs = manager.neg(manager.and2(a, b));
    BDD_ID rhs = manager.or2(manager.neg(a), manager.neg(b));
    EXPECT_EQ(lhs, rhs);
}

#endif