/**
 * @file c8/private/debug.h
 * @note NOT EXPORTED
 *
 * Stuff related to debug mode.
 */

#ifndef C8_DEBUG_H
#define C8_DEBUG_H

#include "../chip8.h"

/**
 * @enum C8_DebugState
 * @brief Debug state enum
 */
typedef enum {
    C8_DEBUG_QUIT,
    C8_DEBUG_CONTINUE,
    C8_DEBUG_STEP,
} C8_DebugState;

/**
  * @enum C8_CommandIdentifier
  * @brief Represents command types
  *
  * This enumeration defines all possible debug mode commands.
  */
typedef enum {
    C8_CMD_NONE           = -1,
    C8_CMD_ADD_BREAKPOINT = 0,
    C8_CMD_RM_BREAKPOINT,
    C8_CMD_CONTINUE,
    C8_CMD_NEXT,
    C8_CMD_SET,
    C8_CMD_LOAD,
    C8_CMD_SAVE,
    C8_CMD_PRINT,
    C8_CMD_HELP,
    C8_CMD_QUIT,
    C8_CMD_LOADFLAGS,
    C8_CMD_SAVEFLAGS,
} C8_CommandIdentifier;

/**
 * @enum C8_ArgIdentifier
 * @brief Represents argument types
 *
 * This enumeration defines all possible debug mode arguments.
 */
typedef enum {
    C8_ARG_NONE = -1,
    C8_ARG_SP   = 0,
    C8_ARG_DT,
    C8_ARG_ST,
    C8_ARG_PC,
    C8_ARG_I,
    C8_ARG_VK,
    C8_ARG_STACK,
    C8_ARG_BG,
    C8_ARG_FG,
    C8_ARG_SFONT,
    C8_ARG_BFONT,
    C8_ARG_QUIRKS,
    C8_ARG_V,
    C8_ARG_R,
    C8_ARG_ADDR,
    C8_ARG_FILE,
} C8_ArgIdentifier;

/**
 * @union C8_ArgValue
 * @brief Stores an argument's value (string or int)
 *
 * This union is used to store the value of an argument, which can either be a
 * string (for file paths or names) or an integer (for register values, addresses,
 * etc.).
 *
 * @param s string value
 * @param i int value
 */
typedef union {
    char* s;
    int   i;
} C8_ArgValue;

/**
 * @struct C8_Arg
 * @brief Represents an argument for a command with a type and value.
 *
 * This structure is used to encapsulate an argument's type and its value.
 * It can represent different types of arguments such as registers, memory addresses,
 * or file paths, depending on the command being executed.
 *
 * @param type Argument type
 * @param value Argument value
 */
typedef struct {
    C8_ArgIdentifier type;
    C8_ArgValue      value;
} C8_Arg;

/**
 * @struct C8_Command
 * @brief Represents a command with an ID, argument ID, and associated argument.
 *
 * This structure is used to encapsulate a command's identifier, its argument,
 * and a value to set the argument to specifically for `set` commands.
 *
 * @param id command identifier
 * @param arg `Arg` argument
 * @param setValue value to set `arg.value` to for set commands
 */
typedef struct {
    C8_CommandIdentifier id;
    C8_Arg               arg;
    int                  setValue;
} C8_Command;

/**
 * @brief Debug help string
 */
#define C8_DEBUG_HELP_STRING                                                                       \
    "Available commands:\n\
break [ADDRESS]: Add breakpoint to PC or ADDRESS, if given\n\
rmbreak [ADDRESS]: Remove breakpoint at PC or ADDRESS, if given\n\
continue: Exit debug mode until next breakpoint or completion\n\
help: Print this help string\n\
load PATH: Load program state from PATH\n\
next: Step to the next instruction\n\
print [ATTRIBUTE]: Print current value of ATTRIBUTE\n\
save PATH: Save program state to the given file\n\
set ATTRIBUTE VALUE: Set the given attribute to the given value\n\
quit: Terminate the program\n\
\n\
Available attributes to print:\n\
PC: Program counter\n\
SP: Stack pointer\n\
DT: Delay timer\n\
ST: Sound timer\n\
I:  I address\n\
K:  Register to store next keypress\n\
V[x]:  All register values or value of Vx, if given\n\
stack: All stack values\n\
$[address]: Value at given address\n\
\n\
If no argument is given to print, it will print all of the above attributes\n\
except for address values.\n"

int c8_debug_repl(C8*);
int c8_has_breakpoint(C8*, uint16_t);

#endif
