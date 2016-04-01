#include <iostream>
#include <cstdint>
#include <map>
#include <random>

using namespace std;

map<uint64_t*, int> ptrMap;

// insert some,delete some and then insert some

int main(int argc, char* argv[]) {

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1,6);
    int dice_roll = distribution(generator);

    for(int i = 0; i < 100; i++) {
        uint64_t* ptr = new uint64_t[100*dice_roll];  
        ptr[0] = i;
        ptrMap.insert(std::make_pair(ptr, i));
    }

    // delete some items
    for(auto B = ptrMap.begin(), E = ptrMap.end();B != E;B++){
        if (B->second >= 40 && B->second < 66) {
            cout << "deleting item: " << B->second << "\n";
            ptrMap.erase(B);
        }
    }
    // now do some insertions
    for (int i = 40; i <= 60; i++){
        uint64_t* ptr2 = new uint64_t[100*dice_roll];  
        ptr2[0] = i;
        ptrMap.insert(std::make_pair(ptr2, i));
    }
    // print the new map
    cout << "\nNew Map: \n";
    for(auto B = ptrMap.begin(), E = ptrMap.end();
            B != E; B++) {
        cout << B->first << ": " ;
        cout << B->second << " " << "\n";
    }


    /*
    for(auto B = ptrMap.begin(), E = ptrMap.end();
            B != E; B++) {
        ptrMap.erase(B);
    }
    */

    return 0;
}
