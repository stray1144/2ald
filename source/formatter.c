#include "2ald.h"
#include <string.h>

formatter_result_t formatter_cxreo_format(archiver_t *archiver, settings_t *settings, reo_file_t *output) {
        return (formatter_result_t) {FORMATTER_OK, {0}};
}

bool flat_symbol_write(buffer_t *output, symbol_t *symbol) {
        if(symbol == nullptr) return false;

        symbol->location = output->used;
        buffer_append(output, symbol->data, symbol->size);

        return true;
}

formatter_result_t formatter_flat_format(archiver_t *archiver, settings_t *settings, buffer_t *output) {
        symbol_t *entry = archiver_symbol_search(archiver, settings->entry);

        if(!flat_symbol_write(output, entry)) return (formatter_result_t) {FORMATTER_MISSING_SYMBOL, .string = settings->entry};
        
        for(size_t i = 0; i < archiver->symbol_table.used; i++) {
                symbol_t *symbol = buffer_get(&archiver->symbol_table, i);
                if(symbol == entry) continue; // already written
                
                flat_symbol_write(output, symbol); // unfailable
        }

        for(size_t i = 0; i < archiver->relocation_table.used; i++) {
                relocation_t *relocation = buffer_get(&archiver->relocation_table, i);

                symbol_t *patch = archiver_symbol_search(archiver, relocation->patch);
                if(patch == nullptr) return (formatter_result_t) {FORMATTER_MISSING_SYMBOL, .string = relocation->patch};

                symbol_t *target = archiver_symbol_search(archiver, relocation->target);
                if(target == nullptr) return (formatter_result_t) {FORMATTER_MISSING_SYMBOL, .string = relocation->target};

                if(8 > target->size) return (formatter_result_t) {FORMATTER_BAD_SIZE, {0}};
                if(8 > (target->size - relocation->addend)) return (formatter_result_t) {FORMATTER_BAD_ADDEND, {0}};

                arcx_address_t address = settings->origin + patch->location;

                memcpy((uint8_t *)output->data + target->location + relocation->addend, &address, 8);
        } // TODO: less chunkier code

        return (formatter_result_t) {FORMATTER_OK, {0}};
}
