#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <random>

using namespace std;

unordered_map<string*, uint64_t> ptrMap;

int main(int argc, char* argv[]) {

    for(int i = 0; i < 10; i++) {
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