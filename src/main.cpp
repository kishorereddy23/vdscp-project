//
// Created by Carolina P. Nogueira 2016
//

#include <iostream>
#include <string>
/*
#include "Manager.h"


int main(int argc, char* argv[])
{
    std::cout << "Nothing implemented, yet" << std::endl;
}

*/


// ================== DEMO: visualize ROBDD for f = a AND b ==================
// To use this demo instead of your current main, temporarily comment out your
// existing main() function above and uncomment the code below.
/*

#include "Manager.h"
using namespace ClassProject;

int main() {
    Manager mgr;

    // Create variables
    BDD_ID a = mgr.createVar("a");
    BDD_ID b = mgr.createVar("b");

    // Build a simple function: f = a AND b
    BDD_ID f = mgr.and2(a, b);

    // Visualize ROBDD to DOT file in project root
    mgr.visualizeBDD("and_ab.dot", f);

    return 0;
}

*/

#include "Manager.h"
using namespace ClassProject;

int main() {
    Manager mgr;

    BDD_ID a = mgr.createVar("a");
    BDD_ID b = mgr.createVar("b");
    BDD_ID c = mgr.createVar("c");
    BDD_ID d = mgr.createVar("d");

    BDD_ID a_or_b = mgr.or2(a, b);             // (a + b)
    BDD_ID f = mgr.and2(mgr.and2(a_or_b, c), d);  // (a + b) * c * d

    mgr.visualizeBDD("f_abcd.dot", f);
    return 0;
}
