/*
 * Handle error messages
 */

#if 0
 enum error_ids {
  OK = 0,
  CPU_NOT_SUPPORTED,
  NO_VALID_DIRECTIVE_OR_MNEMONIC,
  NOT_A_VALID_NUMBER,
  NOT_AN_OPERATOR,
  PARANTHESIS_MISSMATCH,
  SYMBOL_NOT_FOUND,
  SYMBOL_ALREADY_EXIST,
  
  ASM_ADDR_IMMEDIATE_TO_BIG,
  ASM_UNEXPECTED_CHARACTER,
  ASM_INVALID_ADDRESSING_MODE,
  ASM_INDIRECT_MODE_INVALID,
};
#endif

unsigned char *error_msgs[] = {
  "No Errors",
  "CPU not supported",
  "Not a valid assembler directive or mnemonic",
  "Not a valid number",
  "Not an operator",
  "Paranthesis missmatch",
  "Symbol not found",
  "Symbol already exists",
  
  "Trying to address an immediate number larger than 255",
  "Found unexpected characted",
  "Invalid addressing mode for this opcode",
  "The indirect mode was specified incorrectly",
};

