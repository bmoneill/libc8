/**
 * @file c8/private/symbol.h
 * @note NOT EXPORTED
 *
 * Stuff for parsing symbols and instructions for encoding CHIP-8 "assembly".
 */

#ifndef C8_SYMBOL_H
#define C8_SYMBOL_H

#include <stdint.h>

#define C8_INSTRUCTION_COUNT     64
#define C8_LABEL_CEILING         64
#define C8_LABEL_IDENTIFIER_SIZE 20
#define C8_SYMBOL_CEILING        64

/* Instruction strings */
#define C8_S_CLS   "CLS"
#define C8_S_RET   "RET"
#define C8_S_JP    "JP"
#define C8_S_CALL  "CALL"
#define C8_S_SE    "SE"
#define C8_S_SNE   "SNE"
#define C8_S_LD    "LD"
#define C8_S_ADD   "ADD"
#define C8_S_OR    "OR"
#define C8_S_AND   "AND"
#define C8_S_SUB   "SUB"
#define C8_S_SHR   "SHR"
#define C8_S_SUBN  "SUBN"
#define C8_S_SHL   "SHL"
#define C8_S_RND   "RND"
#define C8_S_DRW   "DRW"
#define C8_S_SKP   "SKP"
#define C8_S_SKNP  "SKNP"
#define C8_S_XOR   "XOR"
#define C8_S_SCD   "SCD"
#define C8_S_SCR   "SCR"
#define C8_S_SCL   "SCL"
#define C8_S_EXIT  "EXIT"
#define C8_S_LOW   "LOW"
#define C8_S_HIGH  "HIGH"
#define C8_S_JP_V0 "JP V0,"

/* Reserved identifier strings */
#define C8_S_K  "K"
#define C8_S_F  "F"
#define C8_S_B  "B"
#define C8_S_DT "DT"
#define C8_S_ST "ST"
#define C8_S_I  "I"
#define C8_S_IP "[I]"
#define C8_S_DB ".DB"
#define C8_S_DW ".DW"
#define C8_S_DS ".DS"
#define C8_S_HF "HF"
#define C8_S_R  "R"

/**
 * @enum C8_InstructionIdentifier
 * @brief Represents instruction types
 *
 * This enumeration defines all possible CHIP-8 instructions.
 *
 * NOTE: values to be kept in same order as `instructionStrings`
 */
typedef enum {
    C8_I_NULL = -1,
    C8_I_CLS,
    C8_I_RET,
    C8_I_JP,
    C8_I_CALL,
    C8_I_SE,
    C8_I_SNE,
    C8_I_LD,
    C8_I_ADD,
    C8_I_OR,
    C8_I_AND,
    C8_I_SUB,
    C8_I_SHR,
    C8_I_SUBN,
    C8_I_SHL,
    C8_I_RND,
    C8_I_DRW,
    C8_I_SKP,
    C8_I_SKNP,
    C8_I_XOR,
    C8_I_SCD,
    C8_I_SCR,
    C8_I_SCL,
    C8_I_EXIT,
    C8_I_LOW,
    C8_I_HIGH,
    C8_I_JP_V0,
} C8_InstructionIdentifier;

/**
 * @enum C8_SymbolIdentifier
 * @brief Represents symbol types
 *
 * This enumeration defines all symbol types found during the first assembler
 * pass.
 *
 * NOTE: values before label need to be kept in same order as `identifierStrings`
 */
typedef enum {
    C8_SYM_NULL,
    C8_SYM_DT,
    C8_SYM_ST,
    C8_SYM_I,
    C8_SYM_IP,
    C8_SYM_K,
    C8_SYM_F,
    C8_SYM_B,
    C8_SYM_DB,
    C8_SYM_DW,
    C8_SYM_DS,
    C8_SYM_HF,
    C8_SYM_R,
    C8_SYM_LABEL,
    C8_SYM_INT,
    C8_SYM_INT4,
    C8_SYM_INT8,
    C8_SYM_INT12,
    C8_SYM_STRING,
    C8_SYM_V,
    C8_SYM_INSTRUCTION,
    C8_SYM_LABEL_DEFINITION,
} C8_SymbolIdentifier;

/**
 * @struct InstructionFormat
 * @brief Represents a valid instruction format
 *
 * instruction_t's are checked against instruction_format_t's to verify
 * that they will produce valid instructions.
 *
 * @param cmd instruction command
 * @param base base hex value of command (without parameters)
 * @param pcount parameter count
 * @param ptype parameter types
 * @param pmask parameter masks (where to OR parameters to)
 */
typedef struct {
    C8_InstructionIdentifier cmd;
    uint16_t                 base;
    int                      pcount;
    C8_SymbolIdentifier      ptype[3];
    uint16_t                 pmask[3];
} C8_InstructionFormat;

/**
 * @struct C8_Instruction
 * @brief Represents an instruction
 *
 * During the second pass, this structure is used to verify the instruction's
 * validity and generate the bytecode.
 *
 * @param line line number
 * @param cmd instruction command
 * @param pcount parameter count
 * @param ptype parameter types
 * @param p parameter values
 * @param format corresponding `C8_InstructionFormat` (if valid)
 */
typedef struct {
    int                      line;
    C8_InstructionIdentifier cmd;
    int                      pcount;
    C8_SymbolIdentifier      ptype[3];
    int                      p[3];
    C8_InstructionFormat*    format;
} C8_Instruction;

/**
 * @struct C8_Label
 * @brief Represents a label
 *
 * Represents a label with an identifier and byte value
 *
 * @param identifier string identifier
 * @param byte location of the label
 */
typedef struct {
    char identifier[C8_LABEL_IDENTIFIER_SIZE];
    int  byte;
} C8_Label;

/**
 * @struct C8_LabelList
 * @brief Represents a list of labels
 *
 * @param l pointer to first label
 * @param len length of the list
 * @param ceil maximum length of the list
 */
typedef struct {
    C8_Label* l;
    int       len;
    int       ceil;
} C8_LabelList;

/**
 * @struct C8_Symbol
 * @brief Represents a symbol with a type, value, and line number
 *
 * @param type symbol type
 * @param value symbol value
 * @param ln line number
 */
typedef struct {
    C8_SymbolIdentifier type;
    uint16_t            value;
    int                 ln;
} C8_Symbol;

/**
 * @struct C8_SymbolList
 * @brief Represents a symbol with a type, value, and line number
 *
 * @param s pointer to first symbol
 * @param len number of symbols in list
 * @param ceil amount of symbols that can fit in allocated memory
 */
typedef struct {
    C8_Symbol* s;
    int        len;
    int        ceil;
} C8_SymbolList;

int                         c8_build_instruction(C8_Instruction*, C8_SymbolList*, int);
int                         c8_is_comment(const char*);
int                         c8_is_db(const char*);
int                         c8_is_ds(const char*);
int                         c8_is_dw(const char*);
int                         c8_is_instruction(const char*);
int                         c8_is_label_definition(const char*);
int                         c8_is_label(const char*, const C8_LabelList*);
int                         c8_is_register(const char*);
int                         c8_is_reserved_identifier(const char*);
C8_Symbol*                  c8_next_symbol(C8_SymbolList*);
int                         c8_populate_labels(C8_LabelList*);
int                         c8_resolve_labels(C8_SymbolList*, C8_LabelList*);
int                         c8_shift(uint16_t);
int                         c8_substitute_labels(C8_SymbolList*, C8_LabelList*);

extern const char*          c8_instructionStrings[];
extern const char*          c8_identifierStrings[];
extern C8_InstructionFormat c8_formats[];

#endif
