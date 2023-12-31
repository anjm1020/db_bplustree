#include "page.hpp"

#include <cstring>
#include <iostream>

void put2byte(void *dest, uint16_t data) {
    *(uint16_t *)dest = data;
}

uint16_t get2byte(void *dest) {
    return *(uint16_t *)dest;
}

page::page(uint16_t type) {
    hdr.set_num_data(0);
    hdr.set_data_region_off(PAGE_SIZE - 1 - sizeof(page *));
    hdr.set_offset_array((void *)((uint64_t)this + sizeof(slot_header)));
    hdr.set_page_type(type);
}

uint16_t page::get_type() {
    return hdr.get_page_type();
}

uint16_t page::get_record_size(void *record) {
    uint16_t size = *(uint16_t *)record;
    return size;
}

char *page::get_key(void *record) {
    char *key = (char *)((uint64_t)record + sizeof(uint16_t));
    return key;
}

uint64_t page::get_val(void *key) {
    uint64_t val = *(uint64_t *)((uint64_t)key + (uint64_t)strlen((char *)key) + 1);
    return val;
}

void page::set_leftmost_ptr(page *p) {
    leftmost_ptr = p;
}

page *page::get_leftmost_ptr() {
    return leftmost_ptr;
}

uint64_t page::find(char *key) {
    // Please implement this function in project 1.
    uint16_t type = hdr.get_page_type();
    void *offset_array;
    uint16_t off;
    char *curr_key;
    int cmpFlag;

    offset_array = hdr.get_offset_array();
    if (type == LEAF) {
        for (int i = 0; i < hdr.get_num_data(); i++) {
            off = *(uint16_t *)((uint64_t)offset_array + i * 2);
            curr_key = get_key((void *)((uint64_t)this + (uint64_t)off));
            cmpFlag = strcmp(curr_key, key);
            if (!cmpFlag) {
                return get_val((void *)curr_key);
            } else if (cmpFlag > 0) {
                return 0;
            }
        }
        return 0;
    }

    if (type == INTERNAL) {
        page *next = leftmost_ptr;
        for (int i = 0; i < hdr.get_num_data(); i++) {
            off = *(uint16_t *)((uint64_t)offset_array + i * 2);
            curr_key = get_key((void *)((uint64_t)this + (uint64_t)off));
            if (strcmp(curr_key, key) > 0) {
                break;
            }
            next = (page *)get_val((void *)curr_key);
        }
        return (uint64_t)next;
    }

    return 0;
}
bool page::insert(char *key, uint64_t val) {
    uint16_t type = hdr.get_page_type();

    uint16_t off, record_size, last;
    char *curr_key;
    void *data_region, *offset_array;

    offset_array = hdr.get_offset_array();

    record_size = strlen(key) + 1 + sizeof(val) + 2;

    if (is_full(record_size)) return false;

    last = hdr.get_data_region_off();
    last -= sizeof(val) - 1;
    data_region = (void *)((uint64_t)this + (uint64_t)last);
    *((uint64_t *)data_region) = val;

    last -= strlen(key) + 1;
    data_region = (void *)((uint64_t)this + (uint64_t)last);
    memcpy(data_region, key, strlen(key) + 1);

    last -= 2;
    data_region = (void *)((uint64_t)this + (uint64_t)last);
    put2byte(data_region, record_size);

    int idx = -1;
    do {
        idx++;
        off = *(uint16_t *)((uint64_t)offset_array + idx * 2);
        curr_key = get_key((void *)((uint64_t)this + (uint64_t)off));
    } while (idx < hdr.get_num_data() && strcmp(curr_key, key) < 0);

    for (int i = hdr.get_num_data() - 1; i >= idx; i--) {
        put2byte((void *)((uint64_t)offset_array + (uint64_t)((i + 1) * 2)),
                 *(uint16_t *)((uint64_t)offset_array + (uint64_t)(i * 2)));
    }

    put2byte((void *)((uint64_t)offset_array + (uint64_t)(idx * 2)), last);
    last--;
    hdr.set_data_region_off(last);
    hdr.set_num_data(hdr.get_num_data() + 1);
    return true;
}

page *page::split(char *key, uint64_t val, char **parent_key) {
    page *new_page;
    uint16_t off;
    uint32_t medium, idx;
    uint64_t curr_val;
    char *curr_key;
    void *offset_array;

    offset_array = hdr.get_offset_array();

    idx = -1;
    do {
        idx++;
        off = *(uint16_t *)((uint64_t)offset_array + idx * 2);
        curr_key = get_key((void *)((uint64_t)this + (uint64_t)off));
    } while (idx < hdr.get_num_data() && strcmp(curr_key, key) < 0);

    medium = (hdr.get_num_data() + 1) / 2;

    int diff = 0;
    if (idx < medium) {
        diff = -1;
    }

    new_page = new page(hdr.get_page_type());
    for (int i = medium; i < hdr.get_num_data() + 1; i++) {
        if (i == idx) {
            new_page->insert(key, val);
            diff = -1;
            continue;
        }

        off = *(uint16_t *)((uint64_t)offset_array + (i + diff) * 2);
        curr_key = get_key((void *)((uint64_t)this + (uint64_t)off));
        curr_val = get_val(curr_key);
        new_page->insert(curr_key, curr_val);
    }
    if (idx < medium) {
        hdr.set_num_data((medium - 1) * 2);
        defrag();
        this->insert(key, val);
    } else {
        hdr.set_num_data(hdr.get_num_data() + 1);
        defrag();
    }

    off = *(uint16_t *)((uint64_t)new_page->hdr.get_offset_array());
    *parent_key = get_key((void *)((uint64_t)new_page + (uint64_t)off));
    return new_page;
}

bool page::is_full(uint64_t inserted_record_size) {
    // Please implement this function in project 1.
    uint16_t last = hdr.get_data_region_off();
    uint16_t number_of_data = hdr.get_num_data();

    uint64_t offset_address = (uint64_t)(hdr.get_offset_array()) + (number_of_data + 1) * 2;
    uint64_t inserted_record_least_address = (uint64_t)(this) + last - (inserted_record_size - 1);

    return offset_address > inserted_record_least_address;
}

void page::defrag() {
    page *new_page = new page(get_type());
    int num_data = hdr.get_num_data();
    void *offset_array = hdr.get_offset_array();
    void *stored_key = nullptr;
    uint16_t off = 0;
    uint64_t stored_val = 0;
    void *data_region = nullptr;

    for (int i = 0; i < num_data / 2; i++) {
        off = *(uint16_t *)((uint64_t)offset_array + i * 2);
        data_region = (void *)((uint64_t)this + (uint64_t)off);
        stored_key = get_key(data_region);
        stored_val = get_val((void *)stored_key);
        new_page->insert((char *)stored_key, stored_val);
    }
    new_page->set_leftmost_ptr(get_leftmost_ptr());

    memcpy(this, new_page, sizeof(page));
    hdr.set_offset_array((void *)((uint64_t)this + sizeof(slot_header)));
    delete new_page;
}

void page::print() {
    uint32_t num_data = hdr.get_num_data();
    uint16_t off = 0;
    uint16_t record_size = 0;
    void *offset_array = hdr.get_offset_array();
    void *stored_key = nullptr;
    uint64_t stored_val = 0;

    printf("## slot header\n");
    printf("Number of data :%d\n", num_data);
    printf("offset_array : |");
    for (int i = 0; i < num_data; i++) {
        off = *(uint16_t *)((uint64_t)offset_array + i * 2);
        printf(" %d |", off);
    }
    printf("\n");

    void *data_region = nullptr;
    for (int i = 0; i < num_data; i++) {
        off = *(uint16_t *)((uint64_t)offset_array + i * 2);
        data_region = (void *)((uint64_t)this + (uint64_t)off);
        record_size = get_record_size(data_region);
        stored_key = get_key(data_region);
        stored_val = get_val((void *)stored_key);
        printf("==========================================================\n");
        printf("| data_sz:%u | key: %s | val :%lu |\n", record_size, (char *)stored_key, stored_val,
               strlen((char *)stored_key));
    }
}