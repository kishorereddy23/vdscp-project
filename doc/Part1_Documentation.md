Here’s the full content in one block so you can copy it easily.

Create `doc/Part1_Documentation.md` and paste **everything** below:

```markdown
# VDS Project – Part 1 Documentation

## 1. Overview

This project implements a minimal Binary Decision Diagram (BDD) library in C++, following the algorithms from the “Verification of Digital Systems” lecture by Prof. Kunz.

The core class `Manager` implements the interface `ManagerInterface` and provides:

- creation of Boolean variables and constants (`createVar`, `True`, `False`)
- queries about BDDs (`isConstant`, `isVariable`, `topVar`, `getTopVarName`)
- the core ITE operator (`ite`)
- cofactors with respect to variables (`coFactorTrue`, `coFactorFalse`)
- Boolean operators (`and2`, `or2`, `xor2`, `neg`, `nand2`, `nor2`, `xnor2`)
- structural queries (`findNodes`, `findVars`, `uniqueTableSize`)
- visualization of BDDs as DOT graphs (`visualizeBDD`)

Internally, BDDs are stored as Reduced Ordered BDDs (ROBDDs) in a **unique table**. Each node is uniquely determined by its top variable and its high/low successors. The unique table guarantees:

- no duplicate nodes with the same `(topVar, high, low)` triple
- application of the reduction rule `high == low ⇒ remove node`

All Boolean operations are implemented via the **ITE** operator, as shown in the lecture.

---

## 2. Data Structures

### 2.1 BDD_ID

```
typedef size_t BDD_ID;
```

A `BDD_ID` is an index into the unique table. This keeps the representation simple and efficient.

### 2.2 BDDNode

```
struct BDDNode {
    BDD_ID id;          // index in uniqueTable
    BDD_ID high;        // then-branch
    BDD_ID low;         // else-branch
    BDD_ID topVar;      // top variable of this node
    std::string label;  // variable name (for variable nodes)
};
```

- `id` is equal to the node’s index in `uniqueTable`.
- `high` and `low` are the BDD_IDs of the then/else successors.
- `topVar` is the BDD_ID of the variable that labels this node.
- `label` stores a human-readable variable name (only meaningful for variable nodes).

### 2.3 Unique Table

The `Manager` owns:

```
std::vector<BDDNode> uniqueTable;
BDD_ID trueId;
BDD_ID falseId;
```

Invariants:

- `uniqueTable[0]` = False terminal (ID 0)
- `uniqueTable[1]` = True terminal (ID 1)
- For all other nodes, `id == index in uniqueTable`.

The helper:

```
BDD_ID Manager::findOrCreateNode(BDD_ID high, BDD_ID low, BDD_ID topVar);
```

performs **structural hashing**:

- If `high == low`, it returns `high` directly (reduction).
- Otherwise, it searches the unique table:
  - if a node with the same `(topVar, high, low)` triple exists, it returns its ID;
  - otherwise it creates a new node, appends it to `uniqueTable`, and returns its new ID.

---

## 3. Public Interface and Behavior

This section documents the main functions of `Manager` as required by `ManagerInterface`.

### 3.1 Construction and Constants

```
Manager::Manager();
const BDD_ID &True();
const BDD_ID &False();
```

- The constructor initializes the unique table with:
  - ID 0: False node, with `high = low = topVar = 0`
  - ID 1: True node, with `high = low = topVar = 1`
- `True()` returns the BDD_ID of the True node.
- `False()` returns the BDD_ID of the False node.

### 3.2 Variable Creation

```
BDD_ID createVar(const std::string &label);
```

- Creates a new variable node with a given name.
- The new node has:
  - `high = True`, `low = False`
  - `topVar = newId`
  - `label =` given variable name
- Semantics: the BDD behaves like a single variable:
  - `f(label = 1) = True`
  - `f(label = 0) = False`

Example: `a = createVar("a")` creates a node that represents the Boolean variable `a`.

### 3.3 Basic Queries

```
bool isConstant(BDD_ID f);
bool isVariable(BDD_ID x);
BDD_ID topVar(BDD_ID f);
std::string getTopVarName(const BDD_ID &root);
size_t uniqueTableSize();
```

- `isConstant(f)` returns `true` if `f` is the False or True terminal ID.
- `isVariable(x)` returns `true` if:
  - `x` is not constant, and
  - the node structure matches a variable node (`high = True`, `low = False`).
- `topVar(f)` returns the BDD_ID of the top variable of node `f`.
  - For constants, `topVar(f)` is the node itself (0 or 1).
- `getTopVarName(root)` returns the label of the top variable of the BDD rooted at `root`.
- `uniqueTableSize()` returns the total number of nodes currently stored in the unique table.

---

## 4. Core Operation: ITE

```
BDD_ID ite(BDD_ID i, BDD_ID t, BDD_ID e);
```

The ITE operator implements the ternary function:

\[
\mathrm{ITE}(i,t,e) =
\begin{cases}
t & \text{if } i = 1 \\
e & \text{if } i = 0 \\
\end{cases}
\]

and generalizes to arbitrary BDDs by recursion.

**Terminal cases:**

- If `i == True`, return `t`.
- If `i == False`, return `e`.
- If `(t == True && e == False)`, return `i`.
- If `t == e`, return `t`.

These cases handle simple situations without recursion and match the Boolean identities.

**Recursive case:**

1. Choose the smallest top variable among `i`, `t`, and `e`.
   - Let this variable be `x`.
2. Compute cofactors w.r.t. `x`:
   - `iHigh = coFactorTrue(i, x)`, `iLow = coFactorFalse(i, x)`
   - `tHigh = coFactorTrue(t, x)`, `tLow = coFactorFalse(t, x)`
   - `eHigh = coFactorTrue(e, x)`, `eLow = coFactorFalse(e, x)`
3. Recursively apply ITE:
   - `high = ite(iHigh, tHigh, eHigh)`
   - `low  = ite(iLow,  tLow,  eLow)`
4. Combine results via the unique table:
   - `return findOrCreateNode(high, low, x);`

This algorithm is exactly the one presented in the lecture and is the foundation for all other Boolean operations in this implementation.

---

## 5. Cofactors

### 5.1 Cofactors w.r.t. a Variable

```
BDD_ID coFactorTrue(BDD_ID f, BDD_ID x);
BDD_ID coFactorFalse(BDD_ID f, BDD_ID x);
```

- `coFactorTrue(f, x)` returns the BDD representing `f` under the assignment `x = 1`.
- `coFactorFalse(f, x)` returns the BDD representing `f` under the assignment `x = 0`.

Algorithm:

1. If `f` is constant, return `f`.
2. If `topVar(f) != x`:
   - Recurse on the high and low successors:
     - `highCofactor = coFactorTrue/False(f.high, x)`
     - `lowCofactor  = coFactorTrue/False(f.low,  x)`
   - Rebuild a node with `topVar(f)` and these cofactors using `findOrCreateNode`.
3. If `topVar(f) == x`:
   - For `coFactorTrue`: return the `high` successor of `f`.
   - For `coFactorFalse`: return the `low` successor of `f`.

### 5.2 Cofactors w.r.t. the Top Variable

```
BDD_ID coFactorTrue(BDD_ID f);
BDD_ID coFactorFalse(BDD_ID f);
```

- These versions restrict `f` with respect to its own top variable.
- For non-constant nodes:
  - `coFactorTrue(f)` returns `f.high`.
  - `coFactorFalse(f)` returns `f.low`.
- For constants, the function returns the constant itself.

---

## 6. Boolean Operations via ITE

All Boolean connectives are defined via the ITE operator:

```
BDD_ID and2(BDD_ID a, BDD_ID b);   // ITE(a, b, False)
BDD_ID or2(BDD_ID a, BDD_ID b);    // ITE(a, True, b)
BDD_ID xor2(BDD_ID a, BDD_ID b);   // ITE(a, ¬b, b)
BDD_ID neg(BDD_ID a);              // ITE(a, False, True)
BDD_ID nand2(BDD_ID a, BDD_ID b);  // neg(and2(a, b))
BDD_ID nor2(BDD_ID a, BDD_ID b);   // neg(or2(a, b))
BDD_ID xnor2(BDD_ID a, BDD_ID b);  // neg(xor2(a, b))
```

Interpretation:

- `and2(a, b)`:
  - If `a = 1`, result is `b`.
  - If `a = 0`, result is `0`.
- `or2(a, b)`:
  - If `a = 1`, result is `1`.
  - If `a = 0`, result is `b`.
- `xor2(a, b)`:
  - If `a = 1`, result is `¬b`.
  - If `a = 0`, result is `b`.
- `neg(a)`:
  - If `a = 1`, result is `0`.
  - If `a = 0`, result is `1`.

This matches the standard ITE-based definitions from the lecture.

---

## 7. Structural Queries and Visualization

### 7.1 findNodes

```
void findNodes(const BDD_ID &root, std::set<BDD_ID> &nodes_of_root);
```

- Performs a depth-first search starting from `root`.
- Uses a set to avoid revisiting nodes.
- Inserts all reachable `BDD_ID`s into `nodes_of_root`, including:
  - the root itself,
  - terminal nodes 0 and 1 if they are reachable.

### 7.2 findVars

```
void findVars(const BDD_ID &root, std::set<BDD_ID> &vars_of_root);
```

- First calls `findNodes(root, allNodes)`.
- For each node in `allNodes` that is not constant, it inserts `topVar(node)` into `vars_of_root`.
- The result is the set of all variables that appear in the BDD rooted at `root`.

### 7.3 visualizeBDD

```
void visualizeBDD(std::string filepath, BDD_ID &root);
```

- Writes a DOT graph describing the BDD rooted at `root` to `filepath`.
- Representation:
  - False and True nodes are drawn as boxes labeled `0` and `1`.
  - Variable nodes are drawn as ellipses labeled with their variable name (or `x<id>` if no name is stored).
  - Edges:
    - solid edge from node to its `high` successor (then-branch),
    - dashed edge from node to its `low` successor (else-branch).

The DOT file can be rendered with Graphviz or online DOT viewers to visualize the BDD.

---

## 8. Test Strategy

Unit tests are implemented in `src/test/Tests.h` using GoogleTest. They serve as executable documentation and are organized by functionality.

### 8.1 Basic Manager Behavior

- **TrueAndFalse**: checks that `True()` and `False()` are consistent and distinct.
- **InitialTableSize**: ensures the unique table starts with exactly two nodes (IDs 0 and 1).
- **CreateVariable**: verifies that two different variables get different IDs and that the unique table size grows as expected.

### 8.2 Constants, Variables, and Top Variables

- **IsConstantAndVariable**: checks that constants are recognized by `isConstant` and that created variables are not.
- **IsVariableBranchCoverage**: explicitly tests both branches of `isVariable` (constant vs non-constant).
- **TopVar** and **GetTopVarName**: confirm that `topVar` and `getTopVarName` return the correct variable IDs and labels.

### 8.3 ITE Behavior

- **IteBasic**: tests ITE with constant conditions:
  - `ite(True, a, b)` returns `a`.
  - `ite(False, a, b)` returns `b`.
- **IteActsLikeVariable**: checks that `ite(a, True, False)` behaves like variable `a`.
- **IteTopVarSelection**: constructs examples where the top variable must be chosen from `t` or `e`, exercising internal branch conditions in `ite`.

### 8.4 Boolean Operators

- **AndWithFalse**, **AndWithTrue**:
  - `a ∧ 0 = 0`, `a ∧ 1 = a`.
- **OrWithFalse**, **OrWithTrue**:
  - `a ∨ 0 = a`, `a ∨ 1 = 1`.
- **XorWithSelf**, **XnorWithSelf**:
  - `a ⊕ a = 0`, `a ⊙ a = 1`.
- **NandIsNegAnd**, **NorIsNegOr**, **XnorIsNegXor**:
  - verify that NAND, NOR, and XNOR are implemented as negations of AND, OR, and XOR.

### 8.5 Cofactors and Structure

- **CoFactorTrueFalseOnVariable**:
  - cofactors of a variable w.r.t itself give `True` and `False`.
- **CoFactorTrueFalseOnAnd**:
  - tests cofactors on a composed function `f = a ∧ b` and checks one-argument cofactors.
- **FindNodesSingleVar**, **FindVarsSingleVar**:
  - verify that `findNodes` and `findVars` return the expected sets for a single variable.
- **FindNodesAndVarsOnAnd** and **FindVarsOnCompositeFunction**:
  - similar checks for a composite function `f = a ∧ b`.

### 8.6 Uniqueness and Equivalences

- **And2CreatesAndReusesNode**:
  - calling `and2(a, b)` twice returns the same BDD_ID, demonstrating that `findOrCreateNode` reuses existing nodes in the unique table.
- **DeMorgan**:
  - checks the equivalence `¬(a ∧ b) = ¬a ∨ ¬b`.

### 8.7 Visualization

- **VisualizeBDDSmokeTest**:
  - calls `visualizeBDD` on a simple BDD and checks that the DOT file can be opened, ensuring the function executes successfully.

---

## 9. Summary

Part 1 of the project implements a complete minimal BDD manager:

- BDDs are represented as ROBDDs using a unique table.
- The core operation is the ITE operator, from which all Boolean operations are derived.
- Cofactors support recursive decomposition and analysis of BDDs.
- Structural queries and visualization help understand and debug the BDDs.
- A comprehensive set of GoogleTest tests validates the correctness of each function and provides high line coverage of the implementation.

This documentation, together with the tests, should provide a clear understanding of the implemented functionality for review and feedback.
