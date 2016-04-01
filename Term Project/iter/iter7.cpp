#include <iostream>
#include <cstdint>
#include <map>
#include <random>

using namespace std;

map<string*, int> ptrMap;

int main(int argc, char* argv[]) {
    // create the random Map
    string a = "John";
    string* a1 = &a;
    ptrMap[a1] = 27;

    string b = "Abraham";
    string* b1 = &b;
    ptrMap[b1] = 19;

    string c = "Allison";
    string* c1 = &c;
    ptrMap[c1] = 11;

    string d = "Bam";
    string* d1 = &d;
    ptrMap[d1] = 116;

    string e = "Roshan";
    string* e1 = &e;
    ptrMap[e1] = 9;

    string f = "Sanjeet";
    string* f1 = &f;
    ptrMap[f1] = 12;

    string g = "Tripathy";
    string* g1 = &g;
    ptrMap[g1] = 43;

    string h = "Cat";
    string* h1 = &h;
    ptrMap[h1] = 78;

    string i = "Zu";
    string* i1 = &i;
    ptrMap[i1] = 56;

    string j = "Dog";
    string* j1 = &j;
    ptrMap[j1] = 37;

    for(auto B = ptrMap.begin(), E = ptrMap.end();
        B != E; B++) {
        cout << *B->first << ": ";
        cout << B->second << " " << "\n";
    }

    return 0;
}

// objects allocated on the stack follow a different order
