#include <iostream>
#include <cstdint>
#include <map>
#include <random>
#include <algorithm>

using namespace std;

map<string*, int> ptrMap;

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

string* createRandomString(int i) {
    string *s = new string(random_string());
    return s;
}

int main(int argc, char* argv[]) {
    for (int i = 0; i < 10; i++) {
        string *s = createRandomString(i);
        ptrMap[s] = i;
    }

    for(auto B = ptrMap.begin(), E = ptrMap.end();
        B != E; B++) {
        cout << *B->first << ": ";
        cout << B->second << " " << "\n";
    }

    ptrMap.clear();

    return 0;
}

