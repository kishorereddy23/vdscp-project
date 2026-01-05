#include "Manager.h"
#include <fstream>
#include <algorithm>
#include <iostream>


namespace ClassProject {



///////////////////////////////////////////////////////////////////////////////
// Constructor: initialize unique table with False (0) and True (1)
///////////////////////////////////////////////////////////////////////////////
Manager::Manager() {
    uniqueTable.clear();

    // Constant nodes
    uniqueTable.emplace_back(0, 0, 0, 0, "False");
    falseId = 0;

    uniqueTable.emplace_back(1, 1, 1, 1, "True");
    trueId = 1;

    // Initialize unique-table hash index
    uniqueIndex.clear();
    uniqueIndex.emplace(UniqueKey{0, 0, 0}, falseId);
    uniqueIndex.emplace(UniqueKey{1, 1, 1}, trueId);
}


///////////////////////////////////////////////////////////////////////////////
// Constant nodes
///////////////////////////////////////////////////////////////////////////////
const BDD_ID &Manager::True()  { return trueId; }

const BDD_ID &Manager::False() { return falseId; }

///////////////////////////////////////////////////////////////////////////////
// Basic queries
///////////////////////////////////////////////////////////////////////////////
bool Manager::isConstant(BDD_ID f) {
    return (f == trueId || f == falseId);
}

bool Manager::isVariable(BDD_ID x) {
    if (isConstant(x)) return false;
    const BDDNode &n = uniqueTable[x];
    return (n.topVar == x && n.high == trueId && n.low == falseId);
}

void Manager::debugPrintNode(BDD_ID id) {
    const auto &n = uniqueTable[id];
    std::cout << "id=" << id
              << " topVar=" << n.topVar
              << " high=" << n.high
              << " low=" << n.low
              << " label=" << n.label << "\n";
}


///////////////////////////////////////////////////////////////////////////////
// Variable creation
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::createVar(const std::string &label) {
    BDD_ID id = uniqueTable.size();
    // first create the node
    uniqueTable.emplace_back(id, trueId, falseId, id, label);
    // then register it in the unique index
    UniqueKey key{id, trueId, falseId};
    uniqueIndex.emplace(key, id);
    return id;
}


///////////////////////////////////////////////////////////////////////////////
// Top variable of a node
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::topVar(BDD_ID f) {
    return uniqueTable[f].topVar;
}

std::string Manager::getTopVarName(const BDD_ID &root) {
    BDD_ID top = topVar(root);
    return uniqueTable[top].label;
}

size_t Manager::uniqueTableSize() {
    return uniqueTable.size();
}

///////////////////////////////////////////////////////////////////////////////
// Helper: choose smallest top variable of i, t, e
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::getTopVar(BDD_ID i, BDD_ID t, BDD_ID e) {
    BDD_ID result = topVar(i);
    if (!isConstant(t) && topVar(t) < result) result = topVar(t);
    if (!isConstant(e) && topVar(e) < result) result = topVar(e);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Helper: unique table lookup 
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::findOrCreateNode(BDD_ID high, BDD_ID low, BDD_ID topVariable) {
    if (high == low) return high;

    UniqueKey key{topVariable, high, low};
    auto it = uniqueIndex.find(key);
    if (it != uniqueIndex.end()) {
        return it->second;
    }

    BDD_ID newId = uniqueTable.size();
    uniqueTable.emplace_back(newId, high, low, topVariable, "");
    uniqueIndex.emplace(key, newId);
    return newId;
}


///////////////////////////////////////////////////////////////////////////////
// ITE operator: if i then t else e
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::ite(BDD_ID i, BDD_ID t, BDD_ID e) {
    // Terminal cases (unchanged)
    if (i == trueId)  return t;
    if (i == falseId) return e;
    if (t == trueId && e == falseId) return i;
    if (t == e) return t;

    


    //  memoization lookup
    IteKey key{i, t, e};
    auto it = computedTable.find(key);
    if (it != computedTable.end()) {
        return it->second;
    }

    // Choose top variable (you already do this correctly)
    BDD_ID x = topVar(i);
    if (!isConstant(t) && topVar(t) < x) x = topVar(t);
    if (!isConstant(e) && topVar(e) < x) x = topVar(e);

    // Cofactors
    BDD_ID iHigh = coFactorTrue(i, x);
    BDD_ID iLow  = coFactorFalse(i, x);
    BDD_ID tHigh = coFactorTrue(t, x);
    BDD_ID tLow  = coFactorFalse(t, x);
    BDD_ID eHigh = coFactorTrue(e, x);
    BDD_ID eLow  = coFactorFalse(e, x);

    // Recursive calls
    BDD_ID high = ite(iHigh, tHigh, eHigh);
    BDD_ID low  = ite(iLow,  tLow,  eLow);

    BDD_ID res = findOrCreateNode(high, low, x);

    // NEW: store in computed table
    computedTable.emplace(key, res);

    return res;
}


///////////////////////////////////////////////////////////////////////////////
// Cofactors with respect to variable x
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::coFactorTrue(BDD_ID f, BDD_ID x) {
    if (isConstant(f)) return f;                  // constants unchanged
    if (f == x) return True();                    // x|_{x=1} = 1

    BDD_ID v = topVar(f);
    if (v == x) return uniqueTable[f].high;       // f = ite(x, fh, fl) → fh
    if (v > x) return f;                          // x not in support of f

    BDD_ID h = coFactorTrue(uniqueTable[f].high, x);
    BDD_ID l = coFactorTrue(uniqueTable[f].low,  x);
    return findOrCreateNode(h, l, v);
}



BDD_ID Manager::coFactorFalse(BDD_ID f, BDD_ID x) {
    if (isConstant(f)) return f;                  // constants unchanged
    if (f == x) return False();                   // x|_{x=0} = 0

    BDD_ID v = topVar(f);
    if (v == x) return uniqueTable[f].low;        // f = ite(x, fh, fl) → fl
    if (v > x) return f;                          // x not in support of f

    BDD_ID h = coFactorFalse(uniqueTable[f].high, x);
    BDD_ID l = coFactorFalse(uniqueTable[f].low,  x);
    return findOrCreateNode(h, l, v);
}


///////////////////////////////////////////////////////////////////////////////
// Cofactors ignoring which variable (just go high/low)
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::coFactorTrue(BDD_ID f) {
    if (isConstant(f)) return f;
    return uniqueTable[f].high;
}

BDD_ID Manager::coFactorFalse(BDD_ID f) {
    if (isConstant(f)) return f;
    return uniqueTable[f].low;
}

///////////////////////////////////////////////////////////////////////////////
// Boolean operations via ITE
///////////////////////////////////////////////////////////////////////////////


BDD_ID Manager::or2(BDD_ID a, BDD_ID b) {
    return ite(a, trueId, b);
}

BDD_ID Manager::xor2(BDD_ID a, BDD_ID b) {
    BDD_ID nb = neg(b);
    BDD_ID na = neg(a);
    BDD_ID t1 = and2(a, nb);
    BDD_ID t2 = and2(na, b);
    return or2(t1, t2);
}


BDD_ID Manager::neg(BDD_ID a) {
    // 1) Constants
    if (a == True())  return False();
    if (a == False()) return True();

    // 2) If a is a variable (then branch 1, else branch 0), just flip its children
    if (isVariable(a)) {
        BDD_ID v = topVar(a);
        return findOrCreateNode(False(), True(), v);
    }

    // 3) For composite nodes: structurally negate by negating both children,
    //    topVar stays the same; unique table will give canonical id.
    BDD_ID v   = topVar(a);
    BDD_ID ah  = uniqueTable[a].high;
    BDD_ID al  = uniqueTable[a].low;
    BDD_ID nh  = neg(ah);
    BDD_ID nl  = neg(al);
    return findOrCreateNode(nh, nl, v);
}


BDD_ID Manager::and2(BDD_ID a, BDD_ID b) {
    computedTable.clear();
    return ite(a, b, falseId);
}

BDD_ID Manager::nand2(BDD_ID a, BDD_ID b) {
    return neg(and2(a, b));
}

BDD_ID Manager::nor2(BDD_ID a, BDD_ID b) {
    return neg(or2(a, b));
}

BDD_ID Manager::xnor2(BDD_ID a, BDD_ID b) {
    return neg(xor2(a, b));
}

///////////////////////////////////////////////////////////////////////////////
// Traversal: collect nodes and variables reachable from root
///////////////////////////////////////////////////////////////////////////////
void Manager::findNodes(const BDD_ID &root, std::set<BDD_ID> &nodes_of_root) {
    if (nodes_of_root.find(root) != nodes_of_root.end()) return;
    nodes_of_root.insert(root);
    if (isConstant(root)) return;
    findNodes(uniqueTable[root].high, nodes_of_root);
    findNodes(uniqueTable[root].low, nodes_of_root);
}

void Manager::findVars(const BDD_ID &root, std::set<BDD_ID> &vars_of_root) {
    std::set<BDD_ID> allNodes;
    findNodes(root, allNodes);
    for (BDD_ID node : allNodes) {
        if (!isConstant(node)) {
            vars_of_root.insert(topVar(node));
        }
    }
}




///////////////////////////////////////////////////////////////////////////////
// Visualization: dump BDD as DOT file
///////////////////////////////////////////////////////////////////////////////
void Manager::visualizeBDD(std::string filepath, BDD_ID &root) {
    std::ofstream file(filepath);
    file << "digraph BDD {\n";
    file << "  rankdir=TB;\n";

    std::set<BDD_ID> nodes;
    findNodes(root, nodes);

    for (BDD_ID node : nodes) {
        if (node == falseId) {
            file << "  " << node << " [shape=box, label=\"0\"];\n";
        } else if (node == trueId) {
            file << "  " << node << " [shape=box, label=\"1\"];\n";
        } else {
            std::string label = uniqueTable[topVar(node)].label;
            if (label.empty()) label = "x" + std::to_string(topVar(node));
            file << "  " << node << " [shape=ellipse, label=\"" << label << "\"];\n";
        }
    }

    for (BDD_ID node : nodes) {
        if (!isConstant(node)) {
            file << "  " << node << " -> " << uniqueTable[node].high << " [style=solid];\n";
            file << "  " << node << " -> " << uniqueTable[node].low  << " [style=dashed];\n";
        }
    }

    file << "}\n";
    file.close();
}

} // namespace ClassProject
