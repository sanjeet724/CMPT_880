#include <iostream>
#include <cstdint>
#include <unordered_set>

using namespace std;

unordered_set<string*> ptrset;

int main(int argc, char* argv[]) {

    for(int i = 0; i < 10; i++) {
        string *name  = new string("sName_" + to_string(i));
        ptrset.insert(name);
    }

    for(auto B = ptrset.begin(), E = ptrset.end();
            B != E; B++) {
        cout << *(*B) << "\n";
    }

    ptrset.clear();

    return 0;
}
