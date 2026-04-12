#include "2ald.h"
#include <stdio.h>

bool archiver_init(archiver_t *archiver) {
        if(archiver == nullptr) return false;

        archiver_clear(archiver);

        bool result = true;

        result &= buffer_init(&archiver->files, sizeof(reo_file_t));
        result &= buffer_init(&archiver->representations, sizeof(representation_t));
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
        buffer_clear(&archiver->representations);
        buffer_clear(&archiver->symbol_table);
        buffer_clear(&archiver->relocation_table);
}

representation_t archiver_representation_generate(reo_file_t *file, const char *name) {
        representation_t representation = {0};

        representation.name = name;

        representation.string_size = file->header.sizes[REO_STRING_SECTION]; 
        representation.code_size = file->header.sizes[REO_CODE_SECTION]; 
        representation.data_size = file->header.sizes[REO_DATA_SECTION]; 
        representation.block_size = file->header.sizes[REO_BLOCK_SECTION]; 

        representation.string = buffer_get(&file->strings, 0);
        representation.code = buffer_get(&file->code, 0);
        representation.data = buffer_get(&file->data, 0);

        return representation;
}

void archiver_symbol_extract(archiver_t *archiver, uint32_t source_index, reo_symbol_t *entry) {
        reo_file_t *file = buffer_get(&archiver->files, source_index);
        reo_file_type_t type = reo_type_get(file);

        if(entry->entry.type != REO_ENTRY_SYMBOL) return; 

        representation_t *representation = buffer_get(&archiver->representations, source_index);

        symbol_t symbol = {0};

        symbol.type = (type == REO_TYPE_SHARED) ? SYMBOL_IMPORT : entry->type.location; 
        symbol.source_index = source_index;
        symbol.name = (char *) representation->string + entry->entry.name_string;
        symbol.position = entry->location;

        buffer_append(&archiver->symbol_table, &symbol, 1);

        printf("symbol %s in source[%d] at %s(0x%.16lX)\n", symbol.name, symbol.source_index, symbol_type_names[symbol.type], symbol.position);
}

void archiver_relocation_extract(archiver_t *archiver, uint32_t source_index, reo_relocation_t *entry) {
        if(entry->entry.type != REO_ENTRY_RELOCATION) return; 

        representation_t *representation = buffer_get(&archiver->representations, source_index);

        relocation_t relocation = {0};

        // REO_RELOCATION_ABSOLUTE (0) -> RELOCATION_ABSOLUTE (1)
        // REO_RELOCATION_PC_RELATIVE (1) -> RELOCATION_RIP_RELATIVE (2) 
        // TODO: make prettier code for this 
        relocation.type = entry->type + 1;
        relocation.source_index = source_index;
        relocation.name = (char *) representation->string + entry->entry.name_string;
        relocation.position = entry->patch_location;

        buffer_append(&archiver->relocation_table, &relocation, 1);

        printf("%s %s relocation in source[%d] at code(0x%.16lX)\n", relocation.name, relocation_type_names[relocation.type], relocation.source_index, relocation.position);
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

        representation_t representation = archiver_representation_generate(&file, path);

        uint32_t source_index = archiver->files.used;

        buffer_append(&archiver->files, &file, 1);
        buffer_append(&archiver->representations, &representation, 1);

        for(size_t i = 0; i < reo_entry_count(&file); i++) {
                reo_entry_t *entry = reo_entry_get(&file, i);

                archiver_symbol_extract(archiver, source_index, (void *)entry);
                archiver_relocation_extract(archiver, source_index, (void *)entry);
        }

        return true;
}
