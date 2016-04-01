#include <iostream>
#include <cstdint>
#include <set>

using namespace std;

set<string*> ptrSet;

int main(int argc, char* argv[]) {

    for(int i = 0; i < 10000; i++) {
        string *name  = new string("sName_" + to_string(i));
        ptrSet.insert(name);
    }

    for(auto B = ptrSet.begin(), E = ptrSet.end();
            B != E; B++) {
        cout << *(*B) << "\n";
    }

    for(auto B = ptrSet.begin(), E = ptrSet.end();
            B != E; B++) {
        ptrSet.erase(B);
    }

    return 0;
}

// issue happens with 1000 items on mac : < 10 tries
// issue happens with 10,000 items on mac : < 5 tries