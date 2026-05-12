#include "2ald.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool archiver_init(archiver_t *archiver) {
        if(archiver == nullptr) return false;

        archiver_clear(archiver);

        bool result = true;

        result &= buffer_init(&archiver->files, sizeof(reo_file_t));
        result &= buffer_init(&archiver->symbol_table, sizeof(symbol_t));
        result &= buffer_init(&archiver->relocation_table, sizeof(relocation_t));

        return result;
}

void archiver_clear(archiver_t *archiver) {
        if(archiver == nullptr) return;

        for(size_t i = 0; i < archiver->files.used; i++) {
                reo_file_t *file = buffer_get(&archiver->files, i);
                reo_file_clear(file);
        }

        buffer_clear(&archiver->files);
        buffer_clear(&archiver->symbol_table);
        buffer_clear(&archiver->relocation_table);
}

void *archiver_symbol_data(reo_file_t *file, reo_symbol_t *symbol) {
        void *lut[4] = {nullptr, (void *)(reo_code_get(file) + symbol->location), reo_data_get(file, symbol->location), nullptr};

        return lut[symbol->type.location];
}

symbol_t *archiver_symbol_search(archiver_t *archiver, const char *name) {
        if(name == nullptr) return nullptr;

        for(size_t i = 0; i < archiver->symbol_table.used; i++) {
                symbol_t *symbol = buffer_get(&archiver->symbol_table, i);
                if(strcmp(symbol->name, name) == 0) return symbol;
        }

        return nullptr;
}

void archiver_symbol_extract(archiver_t *archiver, reo_file_t *file, reo_symbol_t *entry) {
        reo_file_type_t type = reo_type_get(file);

        if(entry->entry.type != REO_ENTRY_SYMBOL) return; 

        symbol_t symbol = {0};

        symbol.type = (type == REO_TYPE_SHARED) ? SYMBOL_IMPORT : entry->type.location; 
        symbol.size = entry->symbol_size;
        symbol.name = reo_string_get(file, entry->entry.name_string);
        if(symbol.type != SYMBOL_BLOCK) {
                symbol.data = calloc(1, entry->symbol_size);
                memcpy(symbol.data, archiver_symbol_data(file, entry), entry->symbol_size);
        }

        buffer_append(&archiver->symbol_table, &symbol, 1);
}

void archiver_relocation_extract(archiver_t *archiver, reo_file_t *file, reo_relocation_t *entry) {
        if(entry->entry.type != REO_ENTRY_RELOCATION) return; 

        relocation_t relocation = {0};

        relocation.patch = reo_string_get(file, entry->entry.name_string);
        relocation.target = reo_string_get(file, entry->target_name);
        relocation.addend = entry->addend;

        buffer_append(&archiver->relocation_table, &relocation, 1);
}

bool archiver_load(archiver_t *archiver, const char *path) {
        reo_file_t file = {0};

        reo_file_init(&file);

        if(reo_file_load(&file, path) == false) {
                reo_file_clear(&file);
                return false;
        }

        reo_file_type_t type = reo_type_get(&file);
        if(type == REO_TYPE_NONE || type == REO_TYPE_EXECUTABLE) {
                reo_file_clear(&file);
                return false;
        }

        buffer_append(&archiver->files, &file, 1);

        for(size_t i = 0; i < reo_entry_count(&file); i++) {
                reo_entry_t *entry = reo_entry_get(&file, i);

                archiver_symbol_extract(archiver, &file, (void *)entry);
                archiver_relocation_extract(archiver, &file, (void *)entry);
        }

        return true;
}
