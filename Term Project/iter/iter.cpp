#include <iostream>
#include <cstdint>
#include <set>
#include <random>

using namespace std;

set<uint64_t*> ptrset;

int main(int argc, char* argv[]) {

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1,6);
    int dice_roll = distribution(generator);

    for(int i = 0; i < 100; i++) {
        uint64_t* ptr = new uint64_t[1000*dice_roll];  
        ptr[0] = i;
        ptrset.insert(ptr);
    }

    for(auto B = ptrset.begin(), E = ptrset.end();
            B != E; B++) {
        //cout << (*B)[0] << " ";
        cout << *B << " " << "\n";
    }
    // cout << "\n";


    for(auto B = ptrset.begin(), E = ptrset.end();
            B != E; B++) {
        delete (*B);
    }
    return 0;
}
