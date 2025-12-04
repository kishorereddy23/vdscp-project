#include "Manager.h"
#include <fstream>
#include <algorithm>

namespace ClassProject {

///////////////////////////////////////////////////////////////////////////////
// Constructor: initialize unique table with False (0) and True (1)
///////////////////////////////////////////////////////////////////////////////
Manager::Manager() {
    uniqueTable.clear();
    uniqueTable.emplace_back(0, 0, 0, 0, "False");
    falseId = 0;
    uniqueTable.emplace_back(1, 1, 1, 1, "True");
    trueId = 1;
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
    return (uniqueTable[x].high == trueId && uniqueTable[x].low == falseId);
}

///////////////////////////////////////////////////////////////////////////////
// Variable creation
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::createVar(const std::string &label) {
    BDD_ID newId = uniqueTable.size();
    uniqueTable.emplace_back(newId, trueId, falseId, newId, label);
    return newId;
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
// Helper: unique table lookup / creation
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::findOrCreateNode(BDD_ID high, BDD_ID low, BDD_ID topVariable) {
    if (high == low) return high;

    for (size_t i = 0; i < uniqueTable.size(); ++i) {
        if (uniqueTable[i].high   == high &&
            uniqueTable[i].low    == low  &&
            uniqueTable[i].topVar == topVariable) {
            return i;
        }
    }
    BDD_ID newId = uniqueTable.size();
    uniqueTable.emplace_back(newId, high, low, topVariable, "");
    return newId;
}

///////////////////////////////////////////////////////////////////////////////
// ITE operator: if i then t else e
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::ite(BDD_ID i, BDD_ID t, BDD_ID e) {
    // Terminal cases
    if (i == trueId)  return t;
    if (i == falseId) return e;
    if (t == trueId && e == falseId) return i;
    if (t == e) return t;

    // Choose top variable
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

    // Recursive ITE
    BDD_ID high = ite(iHigh, tHigh, eHigh);
    BDD_ID low  = ite(iLow, tLow, eLow);

    return findOrCreateNode(high, low, x);
}

///////////////////////////////////////////////////////////////////////////////
// Cofactors with respect to variable x
///////////////////////////////////////////////////////////////////////////////
BDD_ID Manager::coFactorTrue(BDD_ID f, BDD_ID x) {
    if (isConstant(f)) return f;
    if (topVar(f) != x) {
        BDD_ID highCofactor = coFactorTrue(uniqueTable[f].high, x);
        BDD_ID lowCofactor  = coFactorTrue(uniqueTable[f].low, x);
        return findOrCreateNode(highCofactor, lowCofactor, topVar(f));
    }
    return uniqueTable[f].high;
}

BDD_ID Manager::coFactorFalse(BDD_ID f, BDD_ID x) {
    if (isConstant(f)) return f;
    if (topVar(f) != x) {
        BDD_ID highCofactor = coFactorFalse(uniqueTable[f].high, x);
        BDD_ID lowCofactor  = coFactorFalse(uniqueTable[f].low, x);
        return findOrCreateNode(highCofactor, lowCofactor, topVar(f));
    }
    return uniqueTable[f].low;
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
BDD_ID Manager::and2(BDD_ID a, BDD_ID b) {
    return ite(a, b, falseId);
}

BDD_ID Manager::or2(BDD_ID a, BDD_ID b) {
    return ite(a, trueId, b);
}

BDD_ID Manager::xor2(BDD_ID a, BDD_ID b) {
    return ite(a, neg(b), b);
}

BDD_ID Manager::neg(BDD_ID a) {
    return ite(a, falseId, trueId);
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
