#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <random>

using namespace std;

unordered_map<uint64_t*, std::string> ptrMap;

int main(int argc, char* argv[]) {

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1,6);
    int dice_roll = distribution(generator);

    for(int i = 0; i < 10; i++) {
        uint64_t* ptr = new uint64_t[10000*dice_roll];  
        ptr[0] = i;
        string name  = "someName_" + to_string(i);
        ptrMap.insert(std::make_pair(ptr, name));
    }

    for(auto B = ptrMap.begin(), E = ptrMap.end();
            B != E; B++) {
        cout << B->first << ": " ;
        cout << B->second << " " << "\n";
    }

    for(auto B = ptrMap.begin(), E = ptrMap.end();
            B != E; B++) {
        ptrMap.erase(B);
    }

    return 0;
}
