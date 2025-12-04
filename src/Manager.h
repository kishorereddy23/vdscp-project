// A minimalistic BDD library, following Wolfgang Kunz lecture slides
//
// Created by Markus Wedler 2014

#ifndef VDSPROJECT_MANAGER_H
#define VDSPROJECT_MANAGER_H

#include "ManagerInterface.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <set>

namespace ClassProject {

    /**
     * @brief Structure representing a single BDD node in the unique table
     */
    struct BDDNode {
        BDD_ID id;          // Unique identifier for this node
        BDD_ID high;        // High successor (then branch)
        BDD_ID low;         // Low successor (else branch)
        BDD_ID topVar;      // Top variable of this node
        std::string label;  // Label/name of the variable (optional)

        BDDNode(BDD_ID id, BDD_ID high, BDD_ID low, BDD_ID topVar, const std::string& label = "")
            : id(id), high(high), low(low), topVar(topVar), label(label) {}
    };

    /**
     * @brief Manager class implementing the BDD operations
     */
    class Manager : public ManagerInterface {
    private:
        std::vector<BDDNode> uniqueTable;
        BDD_ID trueId;
        BDD_ID falseId;

        BDD_ID findOrCreateNode(BDD_ID high, BDD_ID low, BDD_ID topVar);
        BDD_ID getTopVar(BDD_ID i, BDD_ID t, BDD_ID e);

    public:
        Manager();
        ~Manager() = default;

        BDD_ID createVar(const std::string &label) override;
        const BDD_ID &True() override;
        const BDD_ID &False() override;
        bool isConstant(BDD_ID f) override;
        bool isVariable(BDD_ID x) override;
        BDD_ID topVar(BDD_ID f) override;
        BDD_ID ite(BDD_ID i, BDD_ID t, BDD_ID e) override;
        BDD_ID coFactorTrue(BDD_ID f, BDD_ID x) override;
        BDD_ID coFactorFalse(BDD_ID f, BDD_ID x) override;
        BDD_ID coFactorTrue(BDD_ID f) override;
        BDD_ID coFactorFalse(BDD_ID f) override;
        BDD_ID and2(BDD_ID a, BDD_ID b) override;
        BDD_ID or2(BDD_ID a, BDD_ID b) override;
        BDD_ID xor2(BDD_ID a, BDD_ID b) override;
        BDD_ID neg(BDD_ID a) override;
        BDD_ID nand2(BDD_ID a, BDD_ID b) override;
        BDD_ID nor2(BDD_ID a, BDD_ID b) override;
        BDD_ID xnor2(BDD_ID a, BDD_ID b) override;
        std::string getTopVarName(const BDD_ID &root) override;
        void findNodes(const BDD_ID &root, std::set<BDD_ID> &nodes_of_root) override;
        void findVars(const BDD_ID &root, std::set<BDD_ID> &vars_of_root) override;
        size_t uniqueTableSize() override;
        void visualizeBDD(std::string filepath, BDD_ID &root) override;
    };

}

#endif
