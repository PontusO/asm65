/*
 * Attempting to parse some simple assembly instructions
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "global.h"
#include "expr.h"
#include "utils.h"
#include "symbols.h"
#include "errors.h"
#include "output.h"

#define DEBUG
#if defined(DEBUG)
#define A_(x) x
#define PRINT_SYMBOLS
#else
#define A_(x)
#endif

#define MAX_FILENAME_LENGTH   256
#define MAX_LINE_LENGTH       2048

enum cpu_models_id {
  CPUUNDEF,
  CPU6502,
  CPU65C02,
  CPU65C816,
};

struct cpu_models {
  char *cpu;
  int cpu_id;
};

struct cpu_models cm[] = {
  { "6502", CPU6502 },
  { "65C02", CPU65C02 },
  { "65C816", CPU65C816 },
  { NULL, 0 },
};

/*
 * Assembler directive
 */
struct asm_directive {
  char *directive;
  int (*func)(char *buf);
};

/*
 * Assembler mnemonics
 */
struct asm_mnemonic;
struct asm_mnemonic {
  char *mnemonic;
  int (*func)(char *buf, struct asm_mnemonic *am);
  unsigned int amodes;
  unsigned char opcodes[13];
};

/* Assembler directive and mnemonic prototypes */
int dir_cpu(char *buf);
int dir_org(char *buf);
int dir_byte(char *buf);
int dir_word(char *buf);
int dir_dword(char *buf);
int dir_end(char *buf);

int asm_adc(char *buf, struct asm_mnemonic *am);
int asm_and(char *buf, struct asm_mnemonic *am);
int asm_asl(char *buf, struct asm_mnemonic *am);
int asm_bcc(char *buf, struct asm_mnemonic *am);
int asm_bcs(char *buf, struct asm_mnemonic *am);
int asm_beq(char *buf, struct asm_mnemonic *am);
int asm_bit(char *buf, struct asm_mnemonic *am);
int asm_bmi(char *buf, struct asm_mnemonic *am);
int asm_bne(char *buf, struct asm_mnemonic *am);
int asm_bpl(char *buf, struct asm_mnemonic *am);
int asm_brk(char *buf, struct asm_mnemonic *am);
int asm_bvc(char *buf, struct asm_mnemonic *am);
int asm_bvs(char *buf, struct asm_mnemonic *am);
int asm_clc(char *buf, struct asm_mnemonic *am);
int asm_cld(char *buf, struct asm_mnemonic *am);
int asm_cli(char *buf, struct asm_mnemonic *am);
int asm_clv(char *buf, struct asm_mnemonic *am);
int asm_cmp(char *buf, struct asm_mnemonic *am);
int asm_cpx(char *buf, struct asm_mnemonic *am);
int asm_cpy(char *buf, struct asm_mnemonic *am);
int asm_dec(char *buf, struct asm_mnemonic *am);
int asm_dex(char *buf, struct asm_mnemonic *am);
int asm_dey(char *buf, struct asm_mnemonic *am);
int asm_eor(char *buf, struct asm_mnemonic *am);
int asm_inc(char *buf, struct asm_mnemonic *am);
int asm_inx(char *buf, struct asm_mnemonic *am);
int asm_iny(char *buf, struct asm_mnemonic *am);
int asm_jmp(char *buf, struct asm_mnemonic *am);
int asm_jsr(char *buf, struct asm_mnemonic *am);
int asm_lda(char *buf, struct asm_mnemonic *am);
int asm_ldx(char *buf, struct asm_mnemonic *am);
int asm_ldy(char *buf, struct asm_mnemonic *am);
int asm_lsr(char *buf, struct asm_mnemonic *am);
int asm_nop(char *buf, struct asm_mnemonic *am);
int asm_ora(char *buf, struct asm_mnemonic *am);
int asm_pha(char *buf, struct asm_mnemonic *am);
int asm_php(char *buf, struct asm_mnemonic *am);
int asm_pla(char *buf, struct asm_mnemonic *am);
int asm_plp(char *buf, struct asm_mnemonic *am);
int asm_rol(char *buf, struct asm_mnemonic *am);
int asm_ror(char *buf, struct asm_mnemonic *am);
int asm_rti(char *buf, struct asm_mnemonic *am);
int asm_rts(char *buf, struct asm_mnemonic *am);
int asm_sbc(char *buf, struct asm_mnemonic *am);
int asm_sec(char *buf, struct asm_mnemonic *am);
int asm_sed(char *buf, struct asm_mnemonic *am);
int asm_sei(char *buf, struct asm_mnemonic *am);
int asm_sta(char *buf, struct asm_mnemonic *am);
int asm_stx(char *buf, struct asm_mnemonic *am);
int asm_sty(char *buf, struct asm_mnemonic *am);
int asm_tax(char *buf, struct asm_mnemonic *am);
int asm_tay(char *buf, struct asm_mnemonic *am);
int asm_tsx(char *buf, struct asm_mnemonic *am);
int asm_txa(char *buf, struct asm_mnemonic *am);
int asm_txs(char *buf, struct asm_mnemonic *am);
int asm_tya(char *buf, struct asm_mnemonic *am);

struct asm_directive ad[] =
{
  { "CPU", dir_cpu },
  { "ORG", dir_org },
  { "BYTE", dir_byte },
  { "WORD", dir_word },
  { "END", dir_end },
  { NULL, NULL },
};

struct asm_mnemonic am[] =
{
  { "ADC", asm_adc, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX |
                    MODE_ABSOLUTE_IY |
                    MODE_INDIRECT_IX |
                    MODE_INDIRECT_IY }, // .... add with carry
  { "AND", asm_and, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX |
                    MODE_ABSOLUTE_IY |
                    MODE_INDIRECT_IX |
                    MODE_INDIRECT_IY }, // .... and (with accumulator)
  { "ASL", asm_asl, MODE_ACCUMULATOR |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX }, // .... arithmetic shift left
  { "BCC", asm_bcc, MODE_RELATIVE }, // .... branch on carry clear
  { "BCS", asm_bcs, MODE_RELATIVE }, // .... branch on carry set
  { "BEQ", asm_beq, MODE_RELATIVE }, // .... branch on equal (zero set)
  { "BIT", asm_bit, MODE_ZEROPAGE |
                    MODE_ABSOLUTE }, // .... bit test
  { "BMI", asm_bmi, MODE_RELATIVE }, // .... branch on minus (negative set)
  { "BNE", asm_bne, MODE_RELATIVE }, // .... branch on not equal (zero clear)
  { "BPL", asm_bpl, MODE_RELATIVE }, // .... branch on plus (negative clear)
  { "BRK", asm_brk, MODE_IMPLIED }, // .... interrupt
  { "BVC", asm_bvc, MODE_RELATIVE }, // .... branch on overflow clear
  { "BVS", asm_bvs, MODE_RELATIVE }, // .... branch on overflow set
  { "CLC", asm_clc, MODE_IMPLIED }, // .... clear carry
  { "CLD", asm_cld, MODE_IMPLIED }, // .... clear decimal
  { "CLI", asm_cli, MODE_IMPLIED }, // .... clear interrupt disable
  { "CLV", asm_clv, MODE_IMPLIED }, // .... clear overflow
  { "CMP", asm_cmp, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX |
                    MODE_ABSOLUTE_IY |
                    MODE_INDIRECT_IX |
                    MODE_INDIRECT_IY }, // .... compare (with accumulator)
  { "CPX", asm_cpx, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ABSOLUTE }, // .... compare with X
  { "CPY", asm_cpy, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ABSOLUTE }, // .... compare with Y
  { "DEC", asm_dec, MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX }, // .... decrement
  { "DEX", asm_dex, MODE_IMPLIED }, // .... decrement X
  { "DEY", asm_dey, MODE_IMPLIED }, // .... decrement Y
  { "EOR", asm_eor, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX |
                    MODE_ABSOLUTE_IY |
                    MODE_INDIRECT_IX |
                    MODE_INDIRECT_IY }, // .... exclusive or (with accumulator)
  { "INC", asm_inc, MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX }, // .... increment
  { "INX", asm_inx, MODE_IMPLIED }, // .... increment X
  { "INY", asm_iny, MODE_IMPLIED }, // .... increment Y
  { "JMP", asm_jmp, MODE_ABSOLUTE |
                    MODE_INDIRECT }, // .... jump
  { "JSR", asm_jsr, MODE_ABSOLUTE }, // .... jump subroutine
  { "LDA", asm_lda, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX |
                    MODE_ABSOLUTE_IY |
                    MODE_INDIRECT_IX |
                    MODE_INDIRECT_IY,
                    { 0x00, 0xAD, 0xBD, 0xB9, 0xA9, 0x00, 0x00, 
                      0xA1, 0xB1, 0x00, 0xA5, 0xB5, 0x00} }, // .... load accumulator
  { "LDX", asm_ldx, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IY |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IY,
                    { 0x00, 0xAE, 0x00, 0xBE, 0xA2, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0xA6, 0x00, 0xB6} }, // .... load X
  { "LDY", asm_ldy, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX }, // .... load Y
  { "LSR", asm_lsr, MODE_ACCUMULATOR |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX }, // .... logical shift right
  { "NOP", asm_nop, MODE_IMPLIED }, // .... no operation
  { "ORA", asm_ora, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX |
                    MODE_ABSOLUTE_IY |
                    MODE_INDIRECT_IX |
                    MODE_INDIRECT_IY }, // .... or with accumulator
  { "PHA", asm_pha, MODE_IMPLIED }, // .... push accumulator
  { "PHP", asm_php, MODE_IMPLIED }, // .... push processor status (SR)
  { "PLA", asm_pla, MODE_IMPLIED }, // .... pull accumulator
  { "PLP", asm_plp, MODE_IMPLIED }, // .... pull processor status (SR)
  { "ROL", asm_rol, MODE_ACCUMULATOR |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX }, // .... rotate left
  { "ROR", asm_ror, MODE_ACCUMULATOR |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX }, // .... rotate right
  { "RTI", asm_rti, MODE_IMPLIED }, // .... return from interrupt
  { "RTS", asm_rts, MODE_IMPLIED }, // .... return from subroutine
  { "SBC", asm_sbc, MODE_IMMEDIATE |
                    MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX |
                    MODE_ABSOLUTE_IY |
                    MODE_INDIRECT_IX |
                    MODE_INDIRECT_IY }, // .... subtract with carry
  { "SEC", asm_sec, MODE_IMPLIED }, // .... set carry
  { "SED", asm_sed, MODE_IMPLIED }, // .... set decimal
  { "SEI", asm_sei, MODE_IMPLIED }, // .... set interrupt disable
  { "STA", asm_sta, MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE |
                    MODE_ABSOLUTE_IX |
                    MODE_ABSOLUTE_IY |
                    MODE_INDIRECT_IX |
                    MODE_INDIRECT_IY }, // .... store accumulator
  { "STX", asm_stx, MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IY |
                    MODE_ABSOLUTE }, // .... store X
  { "STY", asm_sty, MODE_ZEROPAGE |
                    MODE_ZEROPAGE_IX |
                    MODE_ABSOLUTE }, // .... store Y
  { "TAX", asm_tax, MODE_IMPLIED }, // .... transfer accumulator to X
  { "TAY", asm_tay, MODE_IMPLIED }, // .... transfer accumulator to Y
  { "TSX", asm_tsx, MODE_IMPLIED }, // .... transfer stack pointer to X
  { "TXA", asm_txa, MODE_IMPLIED }, // .... transfer X to accumulator
  { "TXS", asm_txs, MODE_IMPLIED }, // .... transfer X to stack pointer
  { "TYA", asm_tya, MODE_IMPLIED }, // .... transfer Y to accumulator
  { NULL, NULL },
};

/* Variables used */
FILE *src_file;
FILE *lst_file;
FILE *obj_file;
char src_file_name[MAX_FILENAME_LENGTH];
char buf[MAX_LINE_LENGTH];
int cpu = CPUUNDEF;
int PC = 0;
int line = 1;
int pass = 1;

/******************************************************************************
 *                    Assembler directive handlers
 *****************************************************************************/
/* The cpu directive */
int dir_cpu(char *buf)
{
  char cpustr[64];
  int i = -1;

  getarg(cpustr, buf);
  
  /* See if the CPU is found in the table of supported cpu's */
  while(cm[++i].cpu) {
    if (!strncmp(cpustr, cm[i].cpu, strlen(cm[i].cpu))) {
      cpu = cm[i].cpu_id;
      return OK;
    }
  }
  /* CPU not found in table */
  return CPU_NOT_SUPPORTED;
}

/* The org directive */
int dir_org(char *buf) 
{
  char orgarg[256];
  int base = 10;
  int pos = 0;
  
  /* Get the argument for the org directive */
  getarg(orgarg, buf);
  
  /* The first character is significant for determining
     the type of data here */
  if (isdigit(orgarg[0]) || orgarg[0] == '$') {
    /* it's a number, read it */
    if (orgarg[pos] == '$') {
      base = 16;
      pos++;
    }
    PC = strtoul(&orgarg[pos], NULL, base);
  }
  printf("ORG directive set PC to $%x\n", PC);
     
  return 0;
}

int dir_byte(char *buf)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int dir_word(char *buf)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int dir_end(char *buf)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

/******************************************************************************
 *                       Assembler mnemonics
 *****************************************************************************/
int asm_adc(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_and(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_asl(char *buf, struct asm_mnemonic *am)
{
  int error = OK;
  struct address_mode mode;
  
  error = evaluate_address(buf, &mode);
  if (error) {
    goto exit;
  } else {
    /* Check that it is a valid addressing mode */
    if (!(mode.mode & am->amodes)) {
      printf("Not a valid addressing mode\n");
      error = ASM_INVALID_ADDRESSING_MODE;
    } 
  }
exit:
  return error;
}

int asm_bcc(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_bcs(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_beq(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_bit(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_bmi(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_bne(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_bpl(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_brk(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_bvc(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_bvs(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_clc(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_cld(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_cli(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_clv(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_cmp(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_cpx(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_cpy(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_dec(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_dex(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_dey(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_eor(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_inc(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_inx(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_iny(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_jmp(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_jsr(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_lda(char *buf, struct asm_mnemonic *am)
{
  int error = OK;
  struct address_mode mode;
  struct output_descriptor od;
  unsigned char data[4];
  
  od.data = &data[0];
  error = evaluate_address(buf, &mode);
  if (error) {
    goto exit;
  } else {
    /* Check that it is a valid addressing mode */
    printf("Addressing mode %d, value = %d\n", mode.mode, mode.value);
    if (!(mode.mode & am->amodes)) {
      error = ASM_INVALID_ADDRESSING_MODE;
    } else {
      int decmode = mode2dec(mode.mode);
      data[0] = am->opcodes[decmode];
      switch (mode.mode) {
        case MODE_IMMEDIATE:
          od.length = 2;
          data[1] = mode.value;
          output(&od);
          break;
        case MODE_ZEROPAGE:
          break;
        case MODE_ZEROPAGE_IX:
          break;
        case MODE_ABSOLUTE:
          break;
        case MODE_ABSOLUTE_IX:
          break;
        case MODE_ABSOLUTE_IY:
          break;
        case MODE_INDIRECT_IX:
          break;
        case MODE_INDIRECT_IY:
          break;
        default:
          printf("ERROR, an internal error occured, terminating !\n");
          exit(1);
          break;
      }
    }
  }
exit:
  return error;
}

int asm_ldx(char *buf, struct asm_mnemonic *am)
{
  int error = OK;
  struct address_mode mode;
  struct output_descriptor od;
  unsigned char data[4];
  
  od.data = &data[0];
  error = evaluate_address(buf, &mode);
  if (error) {
    goto exit;
  } else {
    /* Check that it is a valid addressing mode */
    printf("Addressing mode %d, value = %d\n", mode.mode, mode.value);
    if (!(mode.mode & am->amodes)) {
      error = ASM_INVALID_ADDRESSING_MODE;
    } else {
      int decmode = mode2dec(mode.mode);
      data[0] = am->opcodes[decmode];
      switch (mode.mode) {
        case MODE_IMMEDIATE:
          od.length = 2;
          data[1] = mode.value;
          output(&od);
          break;
        case MODE_ZEROPAGE:
          break;
        case MODE_ZEROPAGE_IY:
          break;
        case MODE_ABSOLUTE:
          break;
        case MODE_ABSOLUTE_IY:
          break;
        default:
          printf("ERROR, an internal error occured, terminating !\n");
          exit(1);
          break;
      }
    }
  }
exit:
  return error;
}

int asm_ldy(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_lsr(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_nop(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_ora(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_pha(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_php(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_pla(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_plp(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_rol(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_ror(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_rti(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_rts(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_sbc(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_sec(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_sed(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_sei(char *buf, struct asm_mnemonic *am)
{
  int error = OK;
  struct output_descriptor od;
  unsigned char data;
  
  /* Implied addressing mode, we need to make sure no argument is
     specified */
  buf = skip_white(buf);
  if (!isendofline(*buf)) {
    error = ASM_UNEXPECTED_CHARACTER;
  } else {
    od.length = 1;
    od.data = &data;
    output(&od);
  }
  return error;
}

int asm_sta(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_stx(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_sty(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_tax(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_tay(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_tsx(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_txa(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_txs(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}

int asm_tya(char *buf, struct asm_mnemonic *am)
{
  *buf = 0;
  printf("Reached %s !\n", __FUNCTION__);
  return 0;
}


/*
 * Parse current line
 */
static int parse(char *buf)
{
  int i = -1;

  //printf("Parsing: '%s'\n", buf);
  while (ad[++i].directive) {
//    printf ("Checking against %s\n", ad[i].directive);
    if (!strncmp(buf, ad[i].directive, strlen(ad[i].directive))) {
      if (ad[i].func) {
        return ad[i].func((char *)(buf + strlen(ad[i].directive)));
      } else {
        printf("A seriouos error occured in the application, terminating !\n");
        printf("Could not find a function to call for the %s directive.\n", ad[i].directive);
        exit(1);
      }
    }
  }
  /* No assembler directives were found so now we look for assembler mnemonics */
  i = -1;
  while (am[++i].mnemonic) {
    char temp[10];
    /* Convert mnemonic to upper case before comparing */
    strntoupper(temp, buf, 10);
    /* The strlen in the next line assumes that no mnemonic is longer than 10 characters */
    if (!strncmp(temp, am[i].mnemonic, strlen(am[i].mnemonic))) {
      if (am[i].func) {
        return am[i].func((char *)(buf + strlen(am[i].mnemonic)), &am[i]);
      } else {
        printf("A seriouos error occured in the application, terminating !\n");
        printf("Could not find a function to call for the %s mnemonic.\n", am[i].mnemonic);
        exit(1);
      }
    }
  }
  return NO_VALID_DIRECTIVE_OR_MNEMONIC;
}

/*
 * Process incomming line
 */
static int process_line(char *buf)
{
  int error;
  
  /* Check first character to see if we have a
     label defined here. */

  if (isalpha(*buf)) {
    struct symbol_entry *se = read_and_store_label(buf);
    if (!se)
      return SYMBOL_ALREADY_EXIST;
    buf = skiptowhite(buf);
    buf = skip_white(buf);
    /* Check if we have an assignment here */
    if (*buf == '=' || !strncmp(buf, "EQU", 3)) {
      /* Move to next white space character */
      buf = skiptowhite(buf);
      /* And get the value */
      printf ("Evaluating expression %s!\n", buf);
      return eval_expr(buf, NULL, &se->value, 0, NULL);
    }
  }

  buf = skip_white(buf);
  /* Is it a comment or end of line ? */
  if (*buf == ';' || *buf == '\n' || *buf == '\r' || !*buf)
    return;

  return parse(buf);
}

int main (int argc, char **argv)
{
  int error;
  
  printf ("Mag6502 Assembler V0.0001\n");

  if (argc < 2) {
    printf("No source file ! Pls try again.\n");
    exit(1);
  }

  strncpy(src_file_name, argv[1], MAX_FILENAME_LENGTH);
  printf("Assembling source file %s\n", src_file_name);

  src_file =  fopen(src_file_name, "r");
  if (!src_file) {
    printf("Could not find file !\n");
    exit(1);
  }

  /* Initialize the symbol table */
  sym_init();
  
  while (fgets(buf, MAX_LINE_LENGTH, src_file)!= NULL) {
    if ((error = process_line(buf))) {
      printf ("Error %s (error %d), occurred on line %d, terminating execution !\n", 
          error_msgs[error], error, line);
      break;
    }
    line++;
  }

#if defined(PRINT_SYMBOLS)
  {
    struct symbol_entry *se;
    int cnt = num_symbols;
    
    se = se_first;
    
    while (cnt--) {
      printf ("Symbol: %s, name length: %d, Value 0x%x\n", se->symbol_name, se->name_length, se->value);
      se = sym_next_symbol(se);
    }
  }
#endif
  /* Clean up the symbol table */
  sym_clean_up();

  fclose(src_file);
  return 0;
}
