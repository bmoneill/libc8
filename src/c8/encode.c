/**
 * @file c8/encode.c
 *
 * Base assembler code
 */

#include "encode.h"

#include "common.h"
#include "private/exception.h"
#include "private/symbol.h"
#include "private/util.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERBOSE_PRINT(a, ...)                                                                      \
    if (a & C8_ARG_VERBOSE) {                                                                      \
        printf(__VA_ARGS__);                                                                       \
    }

C8_STATIC int   c8_initialize_labels(C8_LabelList*);
C8_STATIC int   c8_initialize_symbols(C8_SymbolList*);
C8_STATIC int   c8_line_count(const char*);
C8_STATIC int   c8_parse_line(char*, int, C8_SymbolList*, const C8_LabelList*);
C8_STATIC int   c8_parse_word(char*, const char*, int, C8_Symbol*, const C8_LabelList*);
C8_STATIC void  c8_put16(uint8_t*, uint16_t, int);
C8_STATIC int   c8_tokenize(char**, char*, const char*, int);
C8_STATIC int   c8_to_upper(char*);
C8_STATIC char* c8_remove_comma(char*);
C8_STATIC int   c8_write(uint8_t*, C8_SymbolList*);

char**          c8_lines;
char**          c8_linesUnformatted;
int             c8_lineCount;

/**
 * @brief Parse the given string
 *
 * This is the main assembler function.
 *
 * This function generates bytecode from the given assembly code.
 *
 * @param s string containing assembly code
 * @param out pointer to write bytecode to
 * @param args command line arguments
 *
 * @return length of resulting bytecode.
 */
int c8_encode(const char* s, uint8_t* out, int args) {
    char* scpy;
    int   len            = strlen(s);
    int   count          = 0;
    int   validLineCount = 0;
    c8_lineCount         = c8_line_count(s);
    C8_LabelList  labels;
    C8_SymbolList symbols;

    c8_initialize_labels(&labels);
    c8_initialize_symbols(&symbols);

    /* copy string and ensure newline at end */
    if (s[len - 1] == '\n') {
        scpy = strndup(s, len);
    } else {
        scpy          = strndup(s, len + 1);
        scpy[len]     = '\n';
        scpy[len + 1] = '\0';
    }

    VERBOSE_PRINT(args, "Getting tokens from input");
    c8_lines     = (char**) malloc(c8_lineCount * sizeof(char*));
    c8_lineCount = c8_tokenize(c8_lines, scpy, "\n", c8_lineCount);

    /*Copy lines to c8_lines_unformatted */
    c8_linesUnformatted = (char**) malloc(c8_lineCount * sizeof(char*));
    for (int i = 0; i < c8_lineCount; i++) {
        c8_linesUnformatted[i] = strdup(c8_lines[i]);
        c8_lines[i]            = c8_trim(c8_lines[i]);
        c8_lines[i]            = c8_remove_comment(c8_lines[i]);
        if (strlen(c8_lines[i]) == 0) {
            // empty line
            c8_lines[i] = NULL;
        } else {
            validLineCount++;
        }
    }

    if (validLineCount == 0) {
        VERBOSE_PRINT(args, "No valid tokens found in input");
        free(scpy);
        free(symbols.s);
        free(labels.l);
        free(c8_lines);
        free(c8_linesUnformatted);
        return 0;
    }

    VERBOSE_PRINT(args, "Populating identifiers in label map\n");
    c8_populate_labels(&labels);

    VERBOSE_PRINT(args, "Populating symbol list\n");
    for (int i = 0; i < c8_lineCount; i++) {
        if (c8_lines[i] == NULL) {
            continue;
        }
        c8_parse_line(c8_lines[i], i + 1, &symbols, &labels);
    }

    VERBOSE_PRINT(args, "Resolving label addresses\n");
    c8_resolve_labels(&symbols, &labels);

    VERBOSE_PRINT(args, "Substituting label addresses in symbol table\n");
    c8_substitute_labels(&symbols, &labels);

    VERBOSE_PRINT(args, "Writing output\n");
    count = c8_write(out, &symbols);

    free(scpy);
    free(symbols.s);
    free(labels.l);
    free(c8_lines);
    free(c8_linesUnformatted);
    return count;
}

/**
 * @brief Remove comment from string if exists
 *
 * @param s string to remove comment from
 * @return string without comment
 */
char* c8_remove_comment(char* s) {
    if (s[0] == ';') {
        s[0] = '\0';
        return s;
    }

    if (strlen(s) < 2) {
        return s;
    }

    for (size_t i = 1; i < strlen(s); i++) {
        if (s[i] == ';' && isspace(s[i - 1])) {
            s[i - 1] = '\0';
        }
    }

    return s;
}

/**
 * @brief Initialize label list
 *
 * @param labels label list to initialize
 *
 * @return 1 if success, exception code otherwise
 */
C8_STATIC int c8_initialize_labels(C8_LabelList* labels) {
    labels->l = (C8_Label*) calloc(C8_LABEL_CEILING, sizeof(C8_Label));
    if (!labels->l) {
        C8_EXCEPTION(C8_MEMORY_ALLOCATION_EXCEPTION, "At function %s", __func__);
        return C8_MEMORY_ALLOCATION_EXCEPTION;
    }

    labels->len  = 0;
    labels->ceil = C8_LABEL_CEILING;
    return 1;
}

/**
 * @brief Initialize symbol list
 *
 * @param symbols symbol list to initialize
 *
 * @return 1 if success, exception code otherwise
 */
C8_STATIC int c8_initialize_symbols(C8_SymbolList* symbols) {
    symbols->s = (C8_Symbol*) calloc(C8_SYMBOL_CEILING, sizeof(C8_Symbol));
    if (!symbols->s) {
        C8_EXCEPTION(C8_MEMORY_ALLOCATION_EXCEPTION, "At function %s", __func__);
        return C8_MEMORY_ALLOCATION_EXCEPTION;
    }

    symbols->len  = 0;
    symbols->ceil = C8_SYMBOL_CEILING;
    return 1;
}

/**
 * @brief Get line count of s
 *
 * @param s string to count lines from
 *
 * @return line count
 */
C8_STATIC int c8_line_count(const char* s) {
    int ln = 1;
    while (*s) {
        if (*s == '\n') {
            ln++;
        }
        s++;
    }
    return ln;
}

/**
 * @brief Generate symbols for the given line
 *
 * @param s line string
 * @param ln line number
 * @param symbols symbol list
 * @param labels label list
 *
 * @return 1 if success, exception code otherwise
 */
C8_STATIC int c8_parse_line(char* s, int ln, C8_SymbolList* symbols, const C8_LabelList* labels) {
    s = c8_trim(s);
    s = c8_remove_comment(s);
    if (strlen(s) == 0) {
        // empty line
        return 1;
    }

    C8_Symbol* sym = c8_next_symbol(symbols);
    char*      words[C8_ENCODE_MAX_WORDS];
    int        wc = c8_tokenize(words, s, " ", C8_ENCODE_MAX_WORDS);

    // Special case for strings
    if (wc > 1 && c8_is_ds(words[0])) {
        for (int i = 1; i < wc; i++) {
            const char* word = words[i];

            if (i > 1) {
                sym->type  = C8_SYM_DB;
                sym->value = ' ';
                sym->ln    = ln;
                sym        = c8_next_symbol(symbols);
            }

            for (int j = i == 1 ? 1 : 0; word[j] != '\0'; j++) {
                if (word[j] == '"' && (i > 1 || j > 0)) {
                    return 1;
                }
                printf("%c", word[j]);
                sym->type  = C8_SYM_DB;
                sym->value = (uint8_t) word[j];
                sym->ln    = ln;
                sym        = c8_next_symbol(symbols);
            }
        }
        return 1;
    }

    for (int i = 0; i < wc; i++) {
        if (i == wc - 1) {
            i += c8_parse_word(words[i], NULL, ln, sym, labels);
        } else {
            i += c8_parse_word(words[i], words[i + 1], ln, sym, labels);
        }
        sym = c8_next_symbol(symbols);
    }

    return 1;
}

/**
 * @brief Generate symbol for the given word
 *
 * @param s word string
 * @param next next word string
 * @param ln line number
 * @param sym symbol to populate
 * @param labels label list
 *
 * @return number of words to skip
 */
C8_STATIC int
c8_parse_word(char* s, const char* next, int ln, C8_Symbol* sym, const C8_LabelList* labels) {
    int value;
    sym->ln = ln;
    s       = c8_remove_comma(s);
    s       = c8_trim(s);
    c8_to_upper(s);

    if (c8_is_label_definition(s) == 1) {
        sym->type = C8_SYM_LABEL_DEFINITION;
        for (int j = 0; j < labels->len; j++) {
            if (!strcmp(s, labels->l[j].identifier)) {
                sym->value = j;
            }
        }
        return 0;
    } else if ((value = c8_is_instruction(s)) >= 0) {
        sym->type  = C8_SYM_INSTRUCTION;
        sym->value = value;
        return 0;
    } else if (c8_is_db(s) && next) {
        sym->type  = C8_SYM_DB;
        sym->value = c8_parse_int(next);
        return 1;
    } else if (c8_is_dw(s) && next) {
        sym->type  = C8_SYM_DW;
        sym->value = c8_parse_int(next);
        return 1;
    } else if ((value = c8_is_register(s)) >= 0) {
        sym->type  = C8_SYM_V;
        sym->value = value;
        return 0;
    } else if ((value = c8_is_reserved_identifier(s)) >= 0) {
        sym->type = (C8_SymbolIdentifier) value;
        return 0;
    } else if ((value = c8_parse_int(s)) > -1) {
        if (value < 0x10) {
            sym->type = C8_SYM_INT4;
        } else if (value < 0x100) {
            sym->type = C8_SYM_INT8;
        } else if (value < 0x1000) {
            sym->type = C8_SYM_INT12;
        } else {
            sym->type = C8_SYM_INT;
        }
        sym->value = value;
        return 0;
    } else if ((value = c8_is_label(s, labels)) >= 0) {
        sym->type  = C8_SYM_LABEL;
        sym->value = value;
        return 0;
    }

    C8_EXCEPTION(C8_INVALID_SYMBOL_EXCEPTION, "Line %d: Invalid symbol '%s'", ln, s);
    return C8_INVALID_SYMBOL_EXCEPTION;
}

/**
 * @brief Write 16 bit int to f
 *
 * @param output where to write
 * @param n index to write to
 */
C8_STATIC void c8_put16(uint8_t* output, uint16_t n, int idx) {
    output[idx]     = (n >> 8) & 0xFF;
    output[idx + 1] = n & 0xFF;
}

/**
 * @brief split string into token array separated by delimeter
 *
 * @param tok token array
 * @param s string to tokenize
 * @param delim delimeter to separate tokens
 * @param maxTokens maximum number of tokens
 *
 * @return number of tokens
 */
C8_STATIC int c8_tokenize(char** tok, char* s, const char* delim, int maxTokens) {
    int   tokenCount = 0;
    char* token      = strtok(s, delim);
    while (token && tokenCount < maxTokens) {
        tok[tokenCount++] = token;
        token             = strtok(NULL, delim);
    }

    return tokenCount;
}

/**
 * @brief Trim and remove comma from s if exists
 *
 * @param s string to remove comma from
 * @return string without comma
 */
C8_STATIC char* c8_remove_comma(char* s) {
    c8_trim(s);
    if (s[strlen(s) - 1] == ',') {
        s[strlen(s) - 1] = '\0';
    }

    return s;
}

/**
 * @brief Convert all characters in null-terminated string s to uppercase
 *
 * @param s string to convert
 */
C8_STATIC int c8_to_upper(char* s) {
    while (*s) {
        *s = toupper(*s);
        s++;
    }
    return 1;
}

/**
 * @brief Convert symbols to bytes and write to output
 *
 * @param output output array
 * @param symbols symbol list
 *
 * @return length of bytecode
 */
C8_STATIC int c8_write(uint8_t* output, C8_SymbolList* symbols) {
    int            ret;
    C8_Instruction ins;
    int            byte = 0;

    for (int i = 0; i < symbols->len; i++) {
        if (byte >= C8_MEMSIZE - C8_PROG_START) {
            return C8_TOO_MANY_SYMBOLS_EXCEPTION;
        }

        int ln = symbols->s[i].ln;
        switch (symbols->s[i].type) {
        case C8_SYM_INSTRUCTION:
            ret = c8_build_instruction(&ins, symbols, i);
            c8_put16(output, ret, byte);
            i += ins.pcount;
            byte += 2;
            break;
        case C8_SYM_DB:
            if (symbols->s[i].value > UINT8_MAX) {
                C8_EXCEPTION(C8_INVALID_ARGUMENT_EXCEPTION,
                             "DB value too big.\nLine %d: %s",
                             ln,
                             c8_linesUnformatted[ln]);
                return C8_INVALID_ARGUMENT_EXCEPTION;
            } else {
                output[byte] = symbols->s[i].value;
                byte++;
            }
            break;
        case C8_SYM_DW:
            c8_put16(output, symbols->s[i].value, byte);
            byte += 2;
            break;
        default:
            break;
        }
    }

    return byte;
}
