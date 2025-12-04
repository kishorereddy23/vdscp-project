#include "Manager.h"
#include <fstream>
#include <algorithm>

namespace ClassProject {

    Manager::Manager() {
    uniqueTable.clear();
    uniqueTable.emplace_back(0, 0, 0, 0, "False");
    falseId = 0;
    uniqueTable.emplace_back(1, 1, 1, 1, "True");
    trueId = 1;
    }

    const BDD_ID &Manager::True() {
        return trueId;
    }

    const BDD_ID &Manager::False() {
        return falseId;
    }

    bool Manager::isConstant(BDD_ID f) {
        return (f == trueId || f == falseId);
    }

    bool Manager::isVariable(BDD_ID x) {
        if (isConstant(x)) return false;
        return (uniqueTable[x].high == trueId && uniqueTable[x].low == falseId);
    }

    BDD_ID Manager::createVar(const std::string &label) {
    BDD_ID newId = uniqueTable.size();
    uniqueTable.emplace_back(newId, trueId, falseId, newId, label);
    return newId;
    }

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

    BDD_ID Manager::getTopVar(BDD_ID i, BDD_ID t, BDD_ID e) {
        BDD_ID result = topVar(i);
        if (!isConstant(i)) result = topVar(i);
        if (!isConstant(t) && topVar(t) < result) result = topVar(t);
        if (!isConstant(e) && topVar(e) < result) result = topVar(e);
        return result;
    }

    BDD_ID Manager::findOrCreateNode(BDD_ID high, BDD_ID low, BDD_ID topVariable) {
        if (high == low) {
            return high;
        }
        for (size_t i = 0; i < uniqueTable.size(); i++) {
            if (uniqueTable[i].high == high && 
                uniqueTable[i].low == low && 
                uniqueTable[i].topVar == topVariable) {
                return i;
            }
        }
        BDD_ID newId = uniqueTable.size();
        uniqueTable.emplace_back(newId, high, low, topVariable, "");
        return newId;
    }

    BDD_ID Manager::coFactorTrue(BDD_ID f, BDD_ID x) {
    if (isConstant(f)) return f;
    if (topVar(f) != x) {
        BDD_ID highCofactor = coFactorTrue(uniqueTable[f].high, x);
        BDD_ID lowCofactor = coFactorTrue(uniqueTable[f].low, x);
        return findOrCreateNode(highCofactor, lowCofactor, topVar(f));
    }
    return uniqueTable[f].high;
    }



    BDD_ID Manager::coFactorFalse(BDD_ID f, BDD_ID x) {
    if (isConstant(f)) return f;
    if (topVar(f) != x) {
        BDD_ID highCofactor = coFactorFalse(uniqueTable[f].high, x);
        BDD_ID lowCofactor = coFactorFalse(uniqueTable[f].low, x);
        return findOrCreateNode(highCofactor, lowCofactor, topVar(f));
    }
    return uniqueTable[f].low;
    }



    BDD_ID Manager::coFactorTrue(BDD_ID f) {
    if (isConstant(f)) return f;
    return uniqueTable[f].high;
    }



    BDD_ID Manager::coFactorFalse(BDD_ID f) {
        if (isConstant(f)) return f;
        return uniqueTable[f].low;
    }

    BDD_ID Manager::ite(BDD_ID i, BDD_ID t, BDD_ID e) {
    // Terminal cases
    if (i == trueId) return t;
    if (i == falseId) return e;
    if (t == trueId && e == falseId) return i;
    if (t == e) return t;

    // Find top variable among i, t, e
    BDD_ID x = topVar(i);
    if (!isConstant(t) && topVar(t) < x) x = topVar(t);
    if (!isConstant(e) && topVar(e) < x) x = topVar(e);

    // Compute cofactors
    BDD_ID iHigh = coFactorTrue(i, x);
    BDD_ID iLow = coFactorFalse(i, x);
    BDD_ID tHigh = coFactorTrue(t, x);
    BDD_ID tLow = coFactorFalse(t, x);
    BDD_ID eHigh = coFactorTrue(e, x);
    BDD_ID eLow = coFactorFalse(e, x);

    // Recursive ITE calls
    BDD_ID high = ite(iHigh, tHigh, eHigh);
    BDD_ID low = ite(iLow, tLow, eLow);

    return findOrCreateNode(high, low, x);
    }



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

    void Manager::findNodes(const BDD_ID &root, std::set<BDD_ID> &nodes_of_root) {
        if (nodes_of_root.find(root) != nodes_of_root.end()) {
            return;
        }
        nodes_of_root.insert(root);
        if (isConstant(root)) {
            return;
        }
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

    void Manager::visualizeBDD(std::string filepath, BDD_ID &root) {
        std::ofstream file(filepath);
        file << "digraph BDD {" << std::endl;
        file << "  rankdir=TB;" << std::endl;
        std::set<BDD_ID> nodes;
        findNodes(root, nodes);
        for (BDD_ID node : nodes) {
            if (node == falseId) {
                file << "  " << node << " [shape=box, label=\"0\"];" << std::endl;
            } else if (node == trueId) {
                file << "  " << node << " [shape=box, label=\"1\"];" << std::endl;
            } else {
                std::string label = uniqueTable[topVar(node)].label;
                if (label.empty()) label = "x" + std::to_string(topVar(node));
                file << "  " << node << " [shape=ellipse, label=\"" << label << "\"];" << std::endl;
            }
        }
        for (BDD_ID node : nodes) {
            if (!isConstant(node)) {
                file << "  " << node << " -> " << uniqueTable[node].high << " [style=solid];" << std::endl;
                file << "  " << node << " -> " << uniqueTable[node].low << " [style=dashed];" << std::endl;
            }
        }
        file << "}" << std::endl;
        file.close();
    }

}
