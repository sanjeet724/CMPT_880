#include <iostream>
#include <cstdint>
#include <map>
#include <random>

using namespace std;

map<string*, int> ptrMap;


int main(int argc, char* argv[]) {

    string Names [] = {"John","Abraham","Allison",
                       "Bam","Roshan","Sanjeet",
                       "Tripathy","Cat","Zu","Dog"};

    for (int i = 0; i < 10 ; i++) {
        string* keyPtr = &Names[i];
        ptrMap[keyPtr] = 23;
    }

    /*
    auto it = ptrMap.find(&Names[4]);
    ptrMap.erase(it);

    string* newKey=&Names[4];
    ptrMap[newKey] = 25;

    string n = "Ramesh";
    string* newkey2 = &n;
    ptrMap[newkey2] = 26;
    */


    for(auto B = ptrMap.begin(), E = ptrMap.end();
        B != E; B++) {
        cout << *B->first << ": ";
        cout << B->second << " " << "\n";
    }

    return 0;
}

// This gives the right order as in the array