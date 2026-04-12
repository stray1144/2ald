// Copyright (C) 2026 Stray1144
// SPDX-License-Identifier: GPL-3.0-or-later

#include "2ald.h"
#include <libvacant/libvacant.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

void context_minimal_clear(context_t *context) {
        if(context == nullptr) return;

        logger_clear(&context->logger);
        argument_parser_clear(&context->AP);

        archiver_clear(&context->archiver);
}

bool context_minimal_init(context_t *context, int argc, char **argv) {
        if(context == nullptr) return false;

        context_minimal_clear(context);
        
        if(logger_init(&context->logger, LOGGER_WARN) == false) {
                printf("Couldn't init the logger...\n");
                return false;
        }

        if(argument_parser_init(&context->AP, argc, argv) == false) {
                system_fatal(context, "parameters", "Couldn't init the argument parser...");
                return false;
        }

        if(archiver_init(&context->archiver) == false) {
                system_fatal(context, "archiver", "Couldn't init the archiver...");
                return false;
        } 

        return true;
}

void shutdown(context_t *context, int exit_code) {
        context_minimal_clear(context);
        exit(exit_code);
}

void help(context_t *context) {
        system_error(context, "help", "not implemented");
}

void settings_handle(context_t *context) {
        if(parameter_probe(&context->AP, "help", PARAMETER_FLAG)) {
                help(context);
                shutdown(context, 0);
        }

        if(parameter_probe(&context->AP, "verbose", PARAMETER_FLAG)) logger_level_change(&context->logger, LOGGER_VERBOSE);
        if(parameter_probe(&context->AP, "debug", PARAMETER_FLAG)) logger_level_change(&context->logger, LOGGER_DEBUG);
        if(parameter_probe(&context->AP, "quiet", PARAMETER_FLAG)) logger_level_change(&context->logger, LOGGER_SILENT);
        if(parameter_probe(&context->AP, "timestamp", PARAMETER_FLAG)) context->settings.logger_timestamp = true;
        context->settings.output_file = parameter_optional_get(&context->AP, "output", "output.cxo");

        char *output_format = parameter_optional_get(&context->AP, "format", "cxreo");
        context->settings.output_format = OUTPUT_UNKNOWN;
        if(strcasecmp(output_format, "cxreo") == 0) context->settings.output_format = OUTPUT_CXREO;
        if(strcasecmp(output_format, "flat") == 0) context->settings.output_format = OUTPUT_FLAT;
}

int main(int argc, char **argv) {
        context_t context = {0};
        if(!context_minimal_init(&context, argc, argv)) shutdown(&context, -1);

        settings_handle(&context);

        if(context.settings.output_format == OUTPUT_UNKNOWN) {
                system_error(&context, "arguments", "Unknown output format");
                shutdown(&context, -1);
        }

        if(parameter_positional_count(&context.AP) == 0) {
                system_error(&context, "file", "Not a single file to link...");
                shutdown(&context, -1);
        }

        for(uint32_t i = 1; i <= parameter_positional_count(&context.AP); i++) {
                char *path = parameter_positional_get(&context.AP, i);

                if(archiver_load(&context.archiver, path) == false) shutdown(&context, -1);
        }

        shutdown(&context, 0);
}
