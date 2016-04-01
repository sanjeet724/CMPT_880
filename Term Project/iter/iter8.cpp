#include <iostream>
#include <cstdint>
#include <map>
#include <random>

using namespace std;

map<string*, int> ptrMap;

int main(int argc, char* argv[]) {
    // create the random Map
    string *a = new string("John");
    ptrMap[a] = 1;

    string *b = new string("Abraham");
    ptrMap[b] = 2;

    string *c = new string("Allison");
    ptrMap[c] = 3;

    string *d = new string("Bam");
    ptrMap[d] = 4;

    string *e = new string("Roshan");
    ptrMap[e] = 5;

    string *f = new string("Sanjeet");
    ptrMap[f] = 6;

    string *g = new string("Tripathy");
    ptrMap[g] = 7;

    string *h = new string("Cat");
    ptrMap[h] = 8;

    string *i = new string("Zu");
    ptrMap[i] = 9;

    string *j = new string("Dog");
    ptrMap[j] = 10;

    // auto it = ptrMap.find(e);
    // ptrMap.erase(it);

    string *k = new string("Ramesh");
    ptrMap[k] = 12;

    delete e;
    // gets inserted at e's place instead of at the end
    string *l = new string("Rahul");
    ptrMap[l] = 12;

    string *m = new string("Mohan");
    ptrMap[m] = 13;
    
    for(auto B = ptrMap.begin(), E = ptrMap.end();
        B != E; B++) {
        cout << *B->first << ": ";
        cout << B->second << " " << "\n";
    }

    return 0;
}

