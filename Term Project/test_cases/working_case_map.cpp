#include <iostream>
#include <cstdint>
#include <map>
#include <random>
#include <algorithm>

using namespace std;

std::string random_string()
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(30,0);
    std::generate_n( str.begin(), 30, randchar );
    return str;
}

string* createRandomString() {
    string *s = new string(random_string());
    return s;
}

uint64_t* createRandomId() {
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1,6);
    int dice_roll = distribution(generator);
    uint64_t* ptr = new uint64_t[100*dice_roll];
    return ptr;
}

class complexObject {
public:
    string *name;
    uint64_t *id;
    uint64_t someArray[10000];
public:
    complexObject(){
        name = createRandomString();
        id = createRandomId();
        for (int i = 0; i < 10000; i++){
            someArray[i] = i;
        }
    }
};

map<complexObject*, int> ptrMap;

int main(int argc, char* argv[]) {
    for (int i = 0; i < 100; i++) {
        complexObject *c = new complexObject();
        ptrMap.insert(std::make_pair(c, i));
    }

    for(auto B = ptrMap.begin(), E = ptrMap.end();
        B != E; B++) {
        cout << *B->first->name << ": ";
        cout << B->second << " " << "\n";
    }

    ptrMap.clear();

    return 0;
}

// issue seen when there are 100 objects in the map
