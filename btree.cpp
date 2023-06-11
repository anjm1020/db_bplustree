#include "btree.hpp"

#include <iostream>

btree::btree() {
    root = new page(LEAF);
    height = 1;
};

void btree::insert(char *key, uint64_t val) {
    // Please implement this function in project 2.
    page *curr_page, *new_page, *new_root;
    char *pkey, *inserted_key;
    uint64_t inserted_val;

    if (root->get_type() == LEAF) {
        // split 판단
        if (!root->insert(key, val)) {
            new_page = root->split(key, val, &pkey);
            new_root = new page(INTERNAL);
            new_root->insert(pkey, (uint64_t)new_page);
            new_root->set_leftmost_ptr(root);
            root = new_root;
            height++;
        }
        return;
    }

    // 일단 leaf 찾기
    page **page_stack = (page **)malloc(sizeof(page *) * height);
    int depth = 0;
    curr_page = root;
    while (curr_page->get_type() != LEAF) {
        page_stack[depth++] = curr_page;
        curr_page = (page *)curr_page->find(key);
    }

    // curr_page는 leaf 인게 보장됨 -> 다시 올라오면서 check
    inserted_key = key;
    inserted_val = val;
    while (!curr_page->insert(inserted_key, inserted_val)) {
        inserted_val = (uint64_t)curr_page->split(inserted_key, inserted_val, &pkey);
        inserted_key = pkey;
        curr_page = page_stack[--depth];
        if (depth < 0) break;
    }

    // 만약 루트까지 올라왔다면? depth<0 -> root split
    if (depth < 0) {
        new_page = root->split(inserted_key, inserted_val, &pkey);
        new_root = new page(INTERNAL);
        new_root->insert(pkey, (uint64_t)new_page);
        new_root->set_leftmost_ptr(root);
        root = new_root;
        height++;
    }
}

uint64_t btree::lookup(char *key) {
    // Please implement this function in project 2.
    page *curr_page;

    curr_page = root;
    while (curr_page->get_type() != LEAF) {
        curr_page = (page *)curr_page->find(key);
    }

    return curr_page->find(key);
}