// Copyright (C) 2026 Stray1144
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdint.h>
#if !defined(__3Asm)
#define __3Asm

#include <cxtoolchain/libcxsh.h>
#include <libvacant/libvacant.h>

#include <sys/types.h>

typedef enum output_format_kind_e {
        OUTPUT_UNKNOWN,
        OUTPUT_CXREO,
        OUTPUT_FLAT
} output_format_kind_t;

typedef uint64_t arcx_address_t;

typedef struct settings_s {
        const char *output_file;
        output_format_kind_t output_format;

        bool logger_timestamp;

        const char *entry;
        arcx_address_t origin;
} settings_t;

typedef enum symbol_type_e {
        SYMBOL_UNKNOWN,
        SYMBOL_CODE,
        SYMBOL_DATA,
        SYMBOL_BLOCK,
        SYMBOL_IMPORT,
        SYMBOL_EMBED
} symbol_type_t;

static char *symbol_type_names[] = {
        "unknown",
        "code",
        "data",
        "block",
        "import",
        "embed"
};

typedef struct symbol_s {
        symbol_type_t type;
        reo_size_t size;
        const char *name;
        arcx_address_t location; // filled by formatter
        void *data;
} symbol_t;

typedef struct relocation_s {
        const char *patch;
        const char *target;
        reo_offset_t addend;
} relocation_t;

typedef struct archiver_s {
        buffer_t files; // buffer_t<reo_file_t>
        buffer_t symbol_table; // buffer_t<symbol_t>
        buffer_t relocation_table; // buffer_t<relocation_t>
} archiver_t;

bool archiver_init(archiver_t *archiver);
void archiver_clear(archiver_t *archiver);

bool archiver_load(archiver_t *archiver, const char *path);

symbol_t *archiver_symbol_search(archiver_t *archiver, const char *name);

typedef enum formatter_status_e {
        FORMATTER_OK,
        FORMATTER_MISSING_SYMBOL,
        FORMATTER_BAD_SIZE,
        FORMATTER_BAD_ADDEND
} formatter_status_t;

typedef struct formatter_result_s {
        formatter_status_t status;
        union {
                const char *string;
        };
} formatter_result_t;

formatter_result_t formatter_cxreo_format(archiver_t *archiver, settings_t *settings, reo_file_t *output);
formatter_result_t formatter_flat_format(archiver_t *archiver, settings_t *settings, buffer_t *output);

typedef struct context_s {
        logger_t logger;
        argument_parser_t AP;
        
        settings_t settings;
        
        archiver_t archiver;
        // formatter_t formatter;
} context_t;

#define SYSTEM_LOGGER(name) system_##name(context_t *context, char *prefix, char *format, ...)

void SYSTEM_LOGGER(fatal); // system_fatal()
void SYSTEM_LOGGER(error); // system_error()
void SYSTEM_LOGGER(warn); // system_warn()
void SYSTEM_LOGGER(info); // system_info()
void SYSTEM_LOGGER(verbose); // system_verbose()
void SYSTEM_LOGGER(debug); // system_debug()

#endif
