#include <algorithm>
#include <cstring>
#include <iostream>

#include "page.hpp"
#define STRING_LEN 5

int main() {
    char key[STRING_LEN];
    char i;
    uint64_t val = 100;
    uint64_t cnt = 0;

    page* p = new page(LEAF);

    p->insert("aaaa", 100);
    p->insert("bbbb", 100);
    p->insert("eeee", 100);
    p->insert("dddd", 100);
    p->insert("ffff", 100);

    char* pkey;
    page* newPage = p->split("cccc", 100, &pkey);

    printf("*** PAGE ***\n");
    p->print();
    printf("*** NEWPAGE ***\n");
    newPage->print();
    printf("*** pkey ***\n");
    ((page*)pkey)->print();

    // val = 100;
    // cnt = 0;
    // for (i = 'j'; i >= 'a'; i--) {
    //     for (int j = 0; j < STRING_LEN - 1; j++) {
    //         key[j] = i;
    //     }
    //     key[STRING_LEN - 1] = '\0';
    //     cnt++;
    //     val *= cnt;
    //     if (val == p->find(key)) {
    //         printf("key :%s founds\n", key);
    //     } else {
    //         printf("key :%s Something wrong\n", key);
    //     }
    // }
    return 0;
}
