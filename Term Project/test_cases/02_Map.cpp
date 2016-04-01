#include <iostream>
#include <cstdint>
#include <map>
#include <random>

using namespace std;

map<string*, uint64_t> ptrMap;

int main(int argc, char* argv[]) {

    for(int i = 0; i < 10000; i++) {
        string *name  = new string("sName_" + to_string(i));
        ptrMap.insert(std::make_pair(name, i));
    }

    for(auto B = ptrMap.begin(), E = ptrMap.end();
            B != E; B++) {
        cout << *B->first << ": " ;
        cout << B->second << " " << "\n";
    }

    ptrMap.clear();

    return 0;
}

// issue replicated with 1000 items on mac : 15-30 tries
// issue replicated with 10,000 items on mac : < 5 tries