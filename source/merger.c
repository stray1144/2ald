#include "2ald.h"
#include <stdio.h>
#include <string.h>

void merger_clear(merger_t *merger) {
        if(merger == nullptr) return;

        for(size_t i = 0; i < merger->file_list.used; i++) reo_file_clear(buffer_get(&merger->file_list, i));
        buffer_clear(&merger->file_list);
}

bool merger_init(merger_t *merger) {
        if(merger == nullptr) return false;

        merger_clear(merger);

        bool status = true;
        status &= buffer_init(&merger->file_list, sizeof(reo_file_t));

        return status;
}

