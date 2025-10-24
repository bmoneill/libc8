/**
 * @file c8/private/symbol.c
 * @note NOT EXPORTED
 *
 * Stuff for parsing symbols and instructions for encoding CHIP-8 "assembly".
 */

#include "symbol.h"

#include "../encode.h"
#include "../defs.h"
#include "exception.h"
#include "util.h"

#include <ctype.h>
#include <string.h>

 /**
  * Reserved identifier strings. Has to match `Symbol`.
  */
const char* c8_identifierStrings[] = {
    "",
    S_DT,
    S_ST,
    S_I,
    S_IP,
    S_K,
    S_F,
    S_B,
    S_DB,
    S_DW,
    S_DS,
    S_HF,
    S_R,
    NULL,
};

/**
 * Instruction strings. Has to match `Instruction`.
 */
const char* c8_instructionStrings[] = {
    S_CLS,
    S_RET,
    S_JP,
    S_CALL,
    S_SE,
    S_SNE,
    S_LD,
    S_ADD,
    S_OR,
    S_AND,
    S_SUB,
    S_SHR,
    S_SUBN,
    S_SHL,
    S_RND,
    S_DRW,
    S_SKP,
    S_SKNP,
    S_XOR,
    S_SCD,
    S_SCR,
    S_SCL,
    S_EXIT,
    S_LOW,
    S_HIGH,
    S_JP_V0,
    NULL,
};

/**
 * All valid instruction formats
 */
instruction_format_t formats[] = {
    { I_SCD,   0x00C0, 1, {SYM_INT4},               {0x000F}},
    { I_CLS,   0x00E0, 0, {SYM_NULL},               {0} },
    { I_RET,   0x00EE, 0, {SYM_NULL},               {0} },
    { I_SCR,   0x00FB, 0, {SYM_NULL},               {0}},
    { I_SCL,   0x00FC, 0, {SYM_NULL},               {0}},
    { I_EXIT,  0x00FD, 0, {SYM_NULL},               {0}},
    { I_LOW,   0x00FE, 0, {SYM_NULL},               {0}},
    { I_HIGH,  0x00FF, 0, {SYM_NULL},               {0}},
    { I_JP,    0x1000, 1, {SYM_INT12},              {0x0FFF} },
    { I_CALL,  0x2000, 1, {SYM_INT12},              {0x0FFF} },
    { I_SE,    0x3000, 2, {SYM_V, SYM_INT8},        {0x0F00, 0x00FF} },
    { I_SNE,   0x4000, 2, {SYM_V, SYM_INT8},        {0x0F00, 0x00FF} },
    { I_SE,    0x5000, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} },
    { I_LD,    0x6000, 2, {SYM_V, SYM_INT8},        {0x0F00, 0x00FF} },
    { I_ADD,   0x7000, 2, {SYM_V, SYM_INT8},        {0x0F00, 0x00FF} },
    { I_LD,    0x8000, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} },
    { I_OR,    0x8001, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} },
    { I_AND,   0x8002, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} },
    { I_XOR,   0x8003, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} },
    { I_ADD,   0x8004, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} },
    { I_SUB,   0x8005, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} },
    { I_SHR,   0x8006, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} }, // must be before below variant for decoding
    { I_SHR,   0x8006, 1, {SYM_V},                  {0x0F00} },
    { I_SUBN,  0x8007, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} },
    { I_SHL,   0x800E, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} }, // must be before below variant for decoding
    { I_SHL,   0x800E, 1, {SYM_V},                  {0x0F00} },
    { I_SNE,   0x9000, 2, {SYM_V, SYM_V},           {0x0F00, 0x00F0} },
    { I_LD,    0xA000, 2, {SYM_I, SYM_INT12},       {0x0000, 0x0FFF} },
    { I_JP_V0, 0xB000, 1, {SYM_INT12},              {0x0FFF} }, // For decoding
    { I_JP,    0xB000, 1, {SYM_V, SYM_INT12},       {0x0000, 0x0FFF} }, // For encoding
    { I_RND,   0xC000, 2, {SYM_V, SYM_INT8},        {0x0F00, 0x00FF} },
    { I_DRW,   0xD000, 3, {SYM_V, SYM_V, SYM_INT4}, {0x0F00, 0x00F0, 0x000F} },
    { I_SKP,   0xE09E, 1, {SYM_V},                  {0x0F00} },
    { I_SKNP,  0xE0A1, 1, {SYM_V},                  {0x0F00} },
    { I_LD,    0xF007, 2, {SYM_V, SYM_DT},          {0x0F00, 0x0000} },
    { I_LD,    0xF00A, 2, {SYM_V, SYM_K},           {0x0F00, 0x0000} },
    { I_LD,    0xF015, 2, {SYM_DT, SYM_V},          {0x0000, 0x0F00} },
    { I_LD,    0xF018, 2, {SYM_ST, SYM_V},          {0x0000, 0x0F00} },
    { I_ADD,   0xF01E, 2, {SYM_I, SYM_V},           {0x0000, 0x0F00} },
    { I_LD,    0xF029, 2, {SYM_F, SYM_V},           {0x0000, 0x0F00} },
    { I_LD,    0xF030, 2, {SYM_HF, SYM_V},          {0x0000, 0x0F00} },
    { I_LD,    0xF033, 2, {SYM_B, SYM_V},           {0x0000, 0x0F00} },
    { I_LD,    0xF055, 2, {SYM_IP, SYM_V},          {0x0000, 0x0F00} },
    { I_LD,    0xF065, 2, {SYM_V, SYM_IP},          {0x0F00, 0x0000} },
    { I_LD,    0xF075, 2, {SYM_R, SYM_V},           {0x0000, 0x0F00} },
    { I_LD,    0xF085, 2, {SYM_V, SYM_R},           {0x0F00, 0x0000} },
    { I_NULL,  0,      0, {SYM_NULL},               {0} },
};

static int get_instruction_args(instruction_t* ins, symbol_list_t* symbols, int idx);
static int parse_instruction(instruction_t*);
static int reallocate_symbols(symbol_list_t* symbols);
static int validate_instruction(instruction_t*);

/**
 * @brief Build an instruction from symbols beginning at idx
 *
 * This function builds an instruction from a completely parsed set of symbols
 * (with labels expanded).
 *
 * @param ins instruction_t to store instruction contents
 * @param symbols symbol list
 * @param idx symbols index of start of instruction
 * @return instruction bytecode
 */
int build_instruction(instruction_t* ins, symbol_list_t* symbols, int idx) {
    ins->cmd = (Instruction)symbols->s[idx].value;
    ins->line = symbols->s[idx].ln;
    ins->pcount = 0;

    get_instruction_args(ins, symbols, idx + 1);
    validate_instruction(ins);
    return parse_instruction(ins);
}

/**
 * @brief Check if the given string is a comment
 *
 * @param s the string to check
 * @return 1 if true, 0 if false
 */
int is_comment(const char* s) {
    return s[0] == ';';
}

/**
 * @brief Check if given string is a DB identifier
 *
 * @return 1 if true, 0 if false
 */
int is_db(const char* s) {
    return !strcmp(s, S_DB);
}

/**
 * @brief Check if given string is a DS identifier
 *
 * @return 1 if true, 0 if false
 */
int is_ds(const char* s) {
    return !strcmp(s, S_DS);
}

/**
 * @brief Check if given string is a DW identifier
 *
 * @return 1 if true, 0 if false
 */
int is_dw(const char* s) {
    return !strcmp(s, S_DW);
}

/**
 * @brief Check if the given string is an instruction
 *
 * @param s the string to check
 * @return instruction enumerator if true, -1 if false
 */
int is_instruction(const char* s) {
    for (int i = 0; c8_instructionStrings[i]; i++) {
        if (!strcmp(s, c8_instructionStrings[i])) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Check if the given string is a label definition
 *
 * @param s the string to check
 * @return 1 if true, 0 if false
 */
int is_label_definition(const char* s) {
    int len = strlen(s);
    if (len < 2) {
        return 0;
    }

    return s[len - 1] == ':';
}

/**
 * @brief Check if given string is a label reference
 *
 * This function checks if the given string is a label reference by
 * comparing it against the label list.
 * It returns the index of the label in the label list
 * if it is found, or -1 if it is not.
 *
 * This function assumes that the label list has been populated
 * (e.g. `populate_labels()` has been called).
 *
 * @param s string to check
 * @param labels label list to check from
 * @return label index if true, -1 otherwise
 */
int is_label(const char* s, const label_list_t* labels) {
    if (strlen(s) == 0) {
        return -1;
    }

    for (int i = 0; i < labels->len; i++) {
        if (!strcmp(s, labels->l[i].identifier)) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Check if the given string represents a V register
 *
 * This function checks if the given string starts with 'V' or 'v'
 * and is followed by a valid hexadecimal digit (0-9, A-F, a-f).
 * It returns the register number if it is a valid V register,
 * or -1 if it is not.
 *
 * @param s string to check
 * @return V register number if true, -1 otherwise
 */
int is_register(const char* s) {
    return (*s == 'V' || *s == 'v') ? parse_int(s) : -1;
}

/**
 * @brief Check if given string is a reserved identifier
 *
 * This function checks if the given string is one of the reserved identifiers
 * defined in `c8_identifierStrings`. It returns the index of the identifier
 * if it is found, or -1 if it is not.
 *
 * @param s string to check
 * @return type of identifier if true, -1 otherwise
 */
int is_reserved_identifier(const char* s) {
    for (int i = 1; c8_identifierStrings[i]; i++) {
        if (!strcmp(s, c8_identifierStrings[i])) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Get the next symbol
 *
 * This function retrieves the next available symbol in the symbol list.
 * If the symbol list is empty, it initializes the first symbol.
 * If the symbol list is full, it reallocates the symbol list to accommodate
 * more symbols.
 *
 * If symbols is `NULL` or the symbol list is `NULL`, it returns `NULL`.
 *
 * @param symbols symbol list to get next symbol from
 *
 * @return first empty symbol in symbol table
 */
symbol_t* next_symbol(symbol_list_t* symbols) {
    if (!symbols || !symbols->s) {
        return NULL;
    }
    if (symbols->len == 0) {
        symbols->len++;
        return &symbols->s[0];
    }

    symbols->len++;
    if (symbols->len >= symbols->ceil) {
        reallocate_symbols(symbols);
    }

    return &symbols->s[symbols->len - 1];
}

/**
 * @brief Populate label list from lines
 *
 * This function scans through the lines of code and identifies label definitions.
 * It checks each line for a label definition (ending with a colon) and adds it
 * to the label list.
 *
 * If a duplicate label definition is found, it throws a `DUPLICATE_LABEL_EXCEPTION`.
 *
 * If too many labels are defined, it throws a `TOO_MANY_LABELS_EXCEPTION`.
 *
 * @param lines lines to search
 * @param lineCount number of lines to search
 * @param labels label list to populate
 *
 * @return 1 if success, 0 if failure
 */
int populate_labels(label_list_t* labels) {
    for (int i = 0; i < c8_line_count; i++) {
        if (strlen(c8_lines[i]) == 0) {
            continue;
        }

        c8_lines[i] = remove_comment(c8_lines[i]);
        trim(c8_lines[i]);
        if (strlen(remove_comment(c8_lines[i])) == 0) {
            continue;
        }

        if (is_label_definition(c8_lines[i])) {
            printf("GOT HERE\n");
            for (int j = 0; j < labels->len; j++) {
                if (!strncmp(labels->l[j].identifier, c8_lines[i], strlen(labels->l[j].identifier))) {
                    C8_EXCEPTION(DUPLICATE_LABEL_EXCEPTION, "Duplicate label definition.\nLine %d: %s", i + 1, c8_lines_unformatted[i + 1]);
                    return DUPLICATE_LABEL_EXCEPTION;
                }
            }

            snprintf(labels->l[labels->len].identifier, LABEL_IDENTIFIER_SIZE - 1, "%s", c8_lines[i]);

            /* remove : */
            int labellen = strlen(c8_lines[i]) - 1;
            labels->l[labels->len].identifier[labellen] = '\0';

            labels->len++;
        }
        if (labels->len == labels->ceil) {
            C8_EXCEPTION(TOO_MANY_LABELS_EXCEPTION, "Too many labels defined in source code.\nLine %d: %s", i + 1, c8_lines[i]);
            return TOO_MANY_LABELS_EXCEPTION;
        }
    }

    return 1;
}

/**
 * @brief Get byte indexes of label definitions from completed symbol table
 *
 * This function resolves label definitions in the symbol list and populates
 * the label list with the byte index of each label definition. It iterates
 * through the symbols, keeping track of the current byte position in the
 * program. When it encounters a label definition, it records the current byte
 * position in the label list. It also increments the byte position based on
 * the type of symbol encountered (e.g., `SYM_DB` increments by 1,
 * `SYM_INSTRUCTION` and `SYM_DW` increment by 2).
 *
 * This function assumes that the symbol list has been populated and that
 * the symbols have been parsed correctly. It also assumes that the label list
 * has been initialized.
 *
 * This function will return 1 if all labels were resolved successfully,
 * or 0 if there was a failure (e.g., if the label list is not fully populated).
 *
 * This function will throw a NULL_ARGUMENT_EXCEPTION if either
 * `symbols` or `labels` is NULL.
 *
 * @param symbols list of symbols
 * @param labels list of labels
 *
 * @return 1 if success, 0 if failure
 */
int resolve_labels(symbol_list_t* symbols, label_list_t* labels) {
    int byte = C8_PROG_START;
    int labelIdx = 0;
    for (int i = 0; i < symbols->len; i++) {
        if (labelIdx == labels->len) {
            return 1;
        }

        switch (symbols->s[i].type) {
        case SYM_LABEL_DEFINITION:
            labels->l[labelIdx++].byte = byte;
            break;
        case SYM_DB:
            byte++;
            break;
        case SYM_INSTRUCTION:
        case SYM_DW:
            byte += 2;
        default:
            break;
        }
    }

    return labelIdx == labels->len;
}

/**
 * @brief Substitute label symbols with their corresponding int value
 *
 * This function replaces all symbols of type `SYM_LABEL` in the symbol list
 * with their corresponding byte value from the label list. It checks if the
 * label exists and throws an `INVALID_SYMBOL_EXCEPTION` if it does not.
 *
 * This function assumes that the label list has been populated and that their
 * byte values have been set correctly.
 *
 * @param symbols symbols to search
 * @param labels labels to search
 */
int substitute_labels(symbol_list_t* symbols, label_list_t* labels) {
    for (int i = 0; i < symbols->len; i++) {
        if (symbols->s[i].type == SYM_LABEL) {
            if (symbols->s[i].value >= labels->len) {
                C8_EXCEPTION(INVALID_SYMBOL_EXCEPTION, "Label does not exist.\nLine %d: %s", symbols->s[i].ln, c8_lines_unformatted[symbols->s[i].ln]);
                return INVALID_SYMBOL_EXCEPTION;
            }
            symbols->s[i].type = SYM_INT12;
            symbols->s[i].value = labels->l[symbols->s[i].value].byte;
        }
    }

    return 1;
}

/**
 * @brief Get instruction arguments from symbols
 *
 * This function populates the instruction's parameters from the symbols
 * starting at the given index. It checks the type of each symbol and assigns
 * the value to the instruction's parameter array.
 *
 * If an integer argument is too large for the expected type, it throws an
 * `INVALID_INSTRUCTION_EXCEPTION`.
 *
 * @param ins instruction to populate
 * @param symbols symbol list
 * @param idx index of first argument in symbols
 *
 * @return 1 if success, exception code otherwise.
 */
static int get_instruction_args(instruction_t* ins, symbol_list_t* symbols, int idx) {
    int j = 0;
    int max = 0;
    for (int i = idx; i < symbols->len; i++) {
        switch (symbols->s[i].type) {
        case SYM_V:
        case SYM_INT12:
            max = 0xFFF;
            // fall through
        case SYM_INT8:
            max = max == 0 ? 0xFF : max;
            // fall through
        case SYM_INT4:
            max = max == 0 ? 0xF : max;
            if (symbols->s[i].value > max) {
                C8_EXCEPTION(INVALID_INSTRUCTION_EXCEPTION,
                    "Line %d: Integer argument too big: %d", symbols->s[i].ln, symbols->s[i].value);
                return INVALID_INSTRUCTION_EXCEPTION;
            }
            ins->p[j] = symbols->s[i].value;
            // fall through
        case SYM_B:
        case SYM_DT:
        case SYM_F:
        case SYM_I:
        case SYM_IP:
        case SYM_K:
        case SYM_ST:
        case SYM_HF:
        case SYM_R:
            ins->ptype[j] = symbols->s[i].type;
            ins->pcount++;
            break;
        default:
            i = symbols->len;
            break;
        }
        j++;
    }
    return 1;
}

/**
 * @brief Get bytecode value of instruction
 *
 * This function takes an instruction and returns its bytecode value.
 * It uses the instruction format to determine how to encode the parameters
 * and combines them with the base value of the instruction format.
 *
 * If ins is `NULL`, it will throw a `NULL_ARGUMENT_EXCEPTION`.
 *
 * @param ins instruction to get bytecode of
 *
 * @return bytecode of instruction ins
 */
static int parse_instruction(instruction_t* ins) {
    uint16_t result = ins->format->base;
    for (int j = 0; j < ins->pcount; j++) {
        if (ins->format->pmask[j]) {
            result |= ins->p[j] << shift(ins->format->pmask[j]);
        }
    }
    return result;
}

/**
 * @brief Validate the given instruction against legal instruction formats
 *
 * If successful, `ins->format` will be populated with the matching format.
 * If the instruction is invalid, an `INVALID_INSTRUCTION_EXCEPTION` will be
 * thrown.
 *
 * @param ins instruction to validate
 *
 * @return 1 if success, 0 if failure
 */
static int validate_instruction(instruction_t* ins) {
    int match;
    for (int i = 0; formats[i].cmd != I_NULL; i++) {
        instruction_format_t* f = &formats[i];
        if (ins->pcount == f->pcount && ins->cmd == f->cmd) {
            match = 1;
            for (int j = 0; j < ins->pcount; j++) {
                switch (ins->ptype[j]) {
                case SYM_INT:
                case SYM_INT4:
                case SYM_INT8:
                case SYM_INT12:
                    if (f->ptype[j] == SYM_INT12 && ins->p[j] < 0x1000) {
                        ins->ptype[j] = SYM_INT12;
                    }
                    else if (f->ptype[j] == SYM_INT8 && ins->p[j] < 0x100) {
                        ins->ptype[j] = SYM_INT8;
                    }
                    else if (f->ptype[j] == SYM_INT4 && ins->p[j] < 0x10) {
                        ins->ptype[j] = SYM_INT4;
                    }
                default:
                    break;
                }

                if (ins->ptype[j] != f->ptype[j]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                ins->format = f;
                return 1;
            }
        }
    }

    C8_EXCEPTION(INVALID_INSTRUCTION_EXCEPTION, "Line %d: %s", ins->line, c8_lines_unformatted[ins->line - 1]);
    return INVALID_INSTRUCTION_EXCEPTION;
}

/**
 * @brief Expand symbol list
 *
 * This function expands the symbol list by allocating more memory for it.
 * It reallocates the symbol list to a block with double the size, copying
 * the existing symbols to the new memory location. This is necessary when
 * the symbol list is full and more symbols need to be added.
 *
 * If symbols is `NULL`, it will return an `NULL_ARGUMENT_EXCEPTION`.
 *
 * @param symbols symbol list
 *
 * @return 1 if success, exception code otherwise.
 */
static int reallocate_symbols(symbol_list_t* symbols) {
    int newCeiling = symbols->ceil + SYMBOL_CEILING;
    symbol_t* oldsym = symbols->s;
    symbols->s = (symbol_t*)malloc(sizeof(symbol_t) * newCeiling);
    memcpy(symbols->s, oldsym, symbols->ceil * sizeof(symbol_t));
    symbols->ceil = newCeiling;
    free(oldsym);

    return 1;
}

/**
 * @brief Find the bits needed to shift to OR a parameter into an instruction
 *
 * This function finds the bits needed to shift to OR a parameter into an instruction.
 * It is used to determine the position of parameters in the instruction bytecode.
 *
 * This is a workaround for the fact that the instruction formats are not
 * defined in a way that allows for easy bit manipulation.
 *
 * @param fmt `instruction_format_t` pformat to check
 *
 * @return number of bits to shift
 */
int shift(uint16_t fmt) {
    static const int table[6][2] = {
        {0xF000, 12}, // a
        {0x000F, 0}, // b
        {0x00FF, 0}, // kk
        {0x0FFF, 0}, // nnn
        {0x0F00, 8}, // x
        {0x00F0, 4}, // y
    };

    for (int i = 0; i < 6; i++) {
        if (fmt == table[i][0]) {
            return table[i][1];
        }
    }

    // Unorthodox shift
    // FIXME remove this?
    int shift = 0;
    while ((fmt & 1) == 0) {
        fmt >>= 1;
        shift++;
    }

    return shift;
}
