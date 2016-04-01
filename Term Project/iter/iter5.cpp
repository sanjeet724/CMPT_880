#include <iostream>
#include <cstdint>
#include <map>
#include <random>

using namespace std;

map<string, int> ptrMap;

int main(int argc, char* argv[]) {

    ptrMap["John"] = 6;
    ptrMap["Abraham"] = 10;
    ptrMap["Allison"] = 23;
    ptrMap["Bam"] = 35;
    ptrMap["Roshan"] = 56;
    ptrMap["Sanjeet"] = 34;
    ptrMap["Tripathy"] = 30;
    ptrMap["Cat"] = 22;
    ptrMap["Zu"] = 11;
    ptrMap["Dog"] = 67;

    for(auto B = ptrMap.begin(), E = ptrMap.end();
        B != E; B++) {
        cout << B->first << ": " ; 
        cout << B->second << " " << "\n";
    }

    return 0;
}
