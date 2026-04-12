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

typedef struct settings_s {
        char *output_file;
        output_format_kind_t output_format;

        bool logger_timestamp;
} settings_t;

typedef uint64_t arcx_address_t;

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
        uint32_t source_index;
        char *name;
        arcx_address_t position;
} symbol_t;

typedef enum relocation_type_e {
        RELOCATION_UNKNOWN,
        RELOCATION_ABSOLUTE,
        RELOCATION_RIP_RELATIVE,
} relocation_type_t;

static char *relocation_type_names[] = {
        "unknown",
        "absolute",
        "rip-relative",
};

typedef struct relocation_s {
        relocation_type_t type;
        uint32_t source_index;
        char *name;
        arcx_address_t position;
} relocation_t;

typedef struct representation_s {
        const char *name;

        reo_size_t string_size;
        reo_size_t code_size;
        reo_size_t data_size;
        reo_size_t block_size;

        uint8_t *string;
        uint8_t *code;
        uint8_t *data;
} representation_t;

typedef struct archiver_s {
        buffer_t files; // buffer_t<reo_file_t>
        buffer_t representations; // buffer_t<representation_t>
        buffer_t symbol_table; // buffer_t<symbol_t>
        buffer_t relocation_table; // buffer_t<relocation_t>
} archiver_t;

bool archiver_init(archiver_t *archiver);
void archiver_clear(archiver_t *archiver);

bool archiver_load(archiver_t *archiver, const char *path);

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

typedef struct encoded_descriptor_s {
	uint8_t register_id : 6;
	uint8_t size : 2;
} encoded_descriptor_t;

#endif
