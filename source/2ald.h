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
        SYMBOL_NORMAL,
        SYMBOL_IMPORT,
        SYMBOL_EMBED
} symbol_type_t;

typedef struct symbol_s {
        symbol_type_t type;
        uint32_t source_index;
        char *name;
        arcx_address_t position;
} symbol_t;

typedef struct representation_s {
        char *name;

        reo_size_t string_size;
        reo_size_t code_size;
        reo_size_t data_size;
        reo_size_t block_size;

        uint8_t *string;
        uint8_t *code;
        uint8_t *data;
} representation_t;

typedef struct archiver_s {
        buffer_t file_content; // buffer_t<reo_file_t>
        buffer_t representations; // buffer_t<representation_t>
        buffer_t symbol_table; // buffer_t<symbol_t>
} archiver_t;

typedef struct context_s {
        logger_t logger;
        argument_parser_t AP;
        
        settings_t settings;
        
        archiver_t archiver;
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
