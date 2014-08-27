/*
 * Handle expressions and evaluate addressing mode
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "global.h"
#include "expr.h"
#include "utils.h"
#include "symbols.h"
#include "errors.h"

#define DEBUG_EXPR
#ifdef DEBUG_EXPR
#define DBG(x) x
#else
#define DBG(x)
#endif

#define MAXOPSTACK 256
#define MAXNUMSTACK 256

int eval_not(int a1, int a2);
int eval_inv(int a1, int a2);
int eval_mul(int a1, int a2);
int eval_div(int a1, int a2);
int eval_mod(int a1, int a2);
int eval_add(int a1, int a2);
int eval_sub(int a1, int a2);
int eval_shift_up(int a1, int a2);
int eval_shift_dn(int a1, int a2);
int eval_and(int a1, int a2);
int eval_exp(int a1, int a2);
int eval_or(int a1, int a2);
int eval_xor(int a1, int a2);
int eval_comma(int a1, int a2);

enum {
  ASSOC_NONE=0, 
  ASSOC_LEFT, 
  ASSOC_RIGHT
};

struct op_s {
  char operator[3];
  int op_id;
  int assoc;
  int level;
  int unary;
  int (*eval)(int a1, int a2);
};

enum op_ids {
  OP_NOT_USED,
  OP_PARANTHESIS_OPEN,
  OP_PARANTHESIS_CLOSE,
  OP_COMMA,
  OP_NOT,
  OP_INV,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_ADD,
  OP_SUB,
  OP_SHIFT_UP,
  OP_SHIFT_DOWN,
  OP_AND,
  OP_EXP,
  OP_OR,
  OP_XOR
};

struct op_s ops[] = {
  { "(", OP_PARANTHESIS_OPEN,  ASSOC_NONE, 20, 0, NULL },
  { ")", OP_PARANTHESIS_CLOSE, ASSOC_NONE, 20, 0, NULL },
  { ",", OP_COMMA,             ASSOC_LEFT,  1, 0, eval_comma },
  { "!", OP_NOT,               ASSOC_RIGHT, 2, 1, eval_not },
  { "~", OP_INV,               ASSOC_RIGHT, 2, 1, eval_inv },
  { "*", OP_MUL,               ASSOC_LEFT,  3, 0, eval_mul },
  { "/", OP_DIV,               ASSOC_LEFT,  3, 0, eval_div },
  { "%", OP_MOD,               ASSOC_LEFT,  3, 0, eval_mod },
  { "+", OP_ADD,               ASSOC_LEFT,  4, 0, eval_add },
  { "-", OP_SUB,               ASSOC_LEFT,  4, 0, eval_sub },
  { "<<", OP_SHIFT_UP,         ASSOC_LEFT,  5, 0, eval_shift_up },
  { ">>", OP_SHIFT_DOWN,       ASSOC_LEFT,  6, 0, eval_shift_dn },
  { "&", OP_AND,               ASSOC_LEFT,  8, 0, eval_and },
  { "^", OP_EXP,               ASSOC_LEFT,  9, 0, eval_exp },
  { "|", OP_OR,                ASSOC_LEFT, 10, 0, eval_or },
  { ":", OP_XOR,               ASSOC_LEFT, 11, 0, eval_xor },
  { "", 0 }
};

struct op_s *opstack[MAXOPSTACK];
int nopstack=0;

int numstack[MAXNUMSTACK];
int nnumstack=0;

/*
 * Helper functions for evaluating expressions
 */
int eval_not(int a1, int a2)
{
  DBG(printf ("!%d = %d\n", a1, !a1));
	return !a1;
}

int eval_inv(int a1, int a2)
{
  DBG(printf ("~%d = %d\n", a1, ~a1));
	return ~a1;
}

int eval_mul(int a1, int a2)
{
  DBG(printf ("%d * %d = %d\n", a1, a2, a1 * a2));
	return a1*a2;
}

int eval_div(int a1, int a2)
{
	if(!a2) {
		fprintf(stderr, "ERROR: Division by zero\n");
		exit(EXIT_FAILURE);
	}
  DBG(printf ("%d / %d = %d\n", a1, a2, a1 / a2));
	return a1 / a2;
}

int eval_mod(int a1, int a2)
{
	if(!a2) {
		fprintf(stderr, "ERROR: Division by zero\n");
		exit(EXIT_FAILURE);
	}
	return a1 % a2;
}

int eval_add(int a1, int a2)
{
  DBG(printf ("%d + %d = %d\n", a1, a2, a1 + a2));
	return a1 + a2;
}

int eval_sub(int a1, int a2)
{
  DBG(printf ("%d - %d = %d\n", a1, a2, a1 - a2));
	return a1 - a2;
}

int eval_shift_up(int a1, int a2)
{
  DBG(printf ("%d << %d = %d\n", a1, a2, a1 << a2));
	return a1 << a2;
}

int eval_shift_dn(int a1, int a2)
{
  DBG(printf ("%d >> %d = %d\n", a1, a2, a1 >> a2));
	return a1 >> a2;
}

int eval_and(int a1, int a2)
{
  DBG(printf ("%d & %d = %d\n", a1, a2, a1 & a2));
	return a1 & a2;
}

int eval_exp(int a1, int a2)
{
	return a2 < 0 ? 0 : (a2 == 0 ? 1 : a1 * eval_exp(a1, a2 - 1));
}

int eval_or(int a1, int a2)
{
  DBG(printf ("%d | %d = %d\n", a1, a2, a1 | a2));
	return a1 | a2;
}

int eval_xor(int a1, int a2)
{
  DBG(printf ("%d ^ %d = %d\n", a1, a2, a1 ^ a2));
	return a1 ^ a2;
}

int eval_comma(int a1, int a2)
{
  DBG(printf ("%d = %d\n", a1, a1));
	return a1;
}

void push_opstack(struct op_s *op)
{
  DBG(printf ("PSH: %s\n", op->operator));
	if(nopstack>MAXOPSTACK-1) {
		fprintf(stderr, "ERROR: Push Operator stack overflow\n");
		exit(EXIT_FAILURE);
	}
	opstack[nopstack++]=op;
}

struct op_s *pop_opstack(void)
{
  DBG(printf ("POP: %s\n", opstack[nopstack-1]->operator));
	if(!nopstack) {
		fprintf(stderr, "ERROR: Pop Operator stack empty\n");
		exit(EXIT_FAILURE);
	}
	return opstack[--nopstack];
}

struct op_s *peek_opstack(void)
{
	if(!nopstack) {
		fprintf(stderr, "ERROR: Peek Operator stack empty\n");
		exit(EXIT_FAILURE);
	}
	return opstack[nopstack-1];
}

void push_numstack(int num)
{
  DBG(printf("PSH: %d\n", num));
	if(nnumstack>MAXNUMSTACK-1) {
		fprintf(stderr, "ERROR: Number stack overflow\n");
		exit(EXIT_FAILURE);
	}
	numstack[nnumstack++]=num;
}

int pop_numstack()
{
  DBG(printf ("POP: %d\n", numstack[nnumstack-1]));
	if(!nnumstack) {
		fprintf(stderr, "ERROR: Number stack empty\n");
		exit(EXIT_FAILURE);
	}
	return numstack[--nnumstack];
}

/*
 * Check if the next section is a valid operator
 */
int is_operator(char *buf, struct op_s **p_ret)
{
  char operator[5];
  struct op_s *op = ops;
  
  memset(operator, 0, sizeof operator);
  getarg(operator, buf);
  /* Look for the operator */
  while (op->level) {
    if (!strncmp(buf, op->operator, strlen(op->operator))) {
      *p_ret = op;
      return 1;
    }
    op++;
  }
  /* No valid operator found */
  return 0;
}

void doop(struct op_s *p)
{
  int a1, a2, r;
  struct op_s *op_se;
  
  /* Peek at the current top entry of the opstack */
  op_se = peek_opstack();
  if (p->assoc == ASSOC_LEFT) {
    while (nopstack && (p->level > op_se->level)){
      op_se = pop_opstack();
      a2 = pop_numstack();
      if (op_se->unary) {
        r = op_se->eval(a2, 0);
      } else {
        a1 = pop_numstack();
        r = op_se->eval(a1, a2);
      }
      push_numstack(r);
      op_se = peek_opstack();
    }
  } else {
    while (nopstack && (p->level >= op_se->level)){
      op_se = pop_opstack();
      a2 = pop_numstack();
      if (op_se->unary) {
        r = op_se->eval(a2, 0);
      } else {
        a1 = pop_numstack();
        r = op_se->eval(a1, a2);
      }
      push_numstack(r);
      op_se = peek_opstack();
    }
  }
  push_opstack(p);
}

/*
 * Evaluate an expression
 */
int eval_expr(char *buf, char **outptr, int *value, int op_expected, int *reg)
{
  struct op_s *p;
  int base = 10;
  int pos = 0;
  int level = 0;
  int tmp;
  int j;
  int a1, a2, r;
  struct op_s *op_se;
  int op_e = 0;

  buf = skip_white(buf);
  DBG(printf ("1:%c\n", *buf));
  /* First check if we are looking at a symbol */
  if (!op_expected && isalpha(*buf)) {
    struct built_in_symbol *bis;
    struct symbol_entry *se = sym_look_for_symbol(buf, &buf);
    if (se) {
      push_numstack(se->value);
    } else {
      /* Check if it is a built in SYMBOL */
      bis = check_built_in_symbol(buf, &buf); 
      if (!bis)
        return SYMBOL_NOT_FOUND;
      else {
        push_numstack(bis->getvalue());
        if (reg)
          *reg = bis->getvalue();
      }
    }
    printf("!!%c\n", *buf);
  } else if (!op_expected && (isdigit(*buf) ||
      *buf == '-' ||
      *buf == '+' ||
      *buf == '$' ||
      *buf == '%' ||
      *buf == '&')) {
    if (*buf == '+' || *buf == '-') {
      if (!isdigit(*(buf+1)))
        return NOT_A_VALID_NUMBER;
    }
    /* it's a number, read it */
    if (*buf == '$') {
      base = 16;
      buf++;
      if (!isxdigit(*buf))
        return NOT_A_VALID_NUMBER;      
    } else if (*buf == '%') {
      base = 2;
      buf++;
      if (!isdigit(*buf))
        return NOT_A_VALID_NUMBER;            
    } else if (*buf == '&') {
      base = 8;
      buf++;
      if (!isdigit(*buf))
        return NOT_A_VALID_NUMBER;            
    }
    errno = 0;
    *value = strtoul(buf, &buf, base);
    if (errno)
      return NOT_A_VALID_NUMBER;
      
    push_numstack(*value);
  }
  
  /* Make it to the next section */
  buf=skip_white(buf);
  DBG(printf ("2:%c - 0x%x\n", *buf, *buf));
  
  /* Check for a valid operator */
  if (is_operator(buf, &p)) {
    DBG(printf("CUR: %s, level %d\n", p->operator, p->level));
    /* yes it was a valid operator */
    /* Peek on the top stack operator (if any) */
    if (nopstack) {
      op_se = peek_opstack();
      DBG(printf("PEK: %s, level %d\n", op_se->operator, op_se->level));
    }
    
    if (p->op_id == OP_PARANTHESIS_OPEN) {
      push_opstack(p);
    } else if (p->op_id == OP_PARANTHESIS_CLOSE) {
      DBG(printf("PAR: Close found.\n"));
      op_e = 1;
      while (nopstack && (peek_opstack()->op_id != OP_PARANTHESIS_OPEN)) {
        op_se = peek_opstack();
        if (p->assoc == ASSOC_LEFT) {
          while (nopstack && (p->level > op_se->level)){
            op_se = pop_opstack();
            a2 = pop_numstack();
            if (op_se->unary) {
              r = op_se->eval(a2, 0);
            } else {
              a1 = pop_numstack();
              r = op_se->eval(a1, a2);
            }
            push_numstack(r);
            op_se = peek_opstack();
          }
        } else {
          while (nopstack && (p->level >= op_se->level)){
            op_se = pop_opstack();
            a2 = pop_numstack();
            if (op_se->unary) {
              r = op_se->eval(a2, 0);
            } else {
              a1 = pop_numstack();
              r = op_se->eval(a1, a2);
            }
            push_numstack(r);
            op_se = peek_opstack();
          }
        }
        push_opstack(p);
      }
      if (nopstack) {
        op_se = pop_opstack();
        if (op_se->op_id != OP_PARANTHESIS_OPEN)
          return PARANTHESIS_MISSMATCH;
      } else
        return PARANTHESIS_MISSMATCH;
    } else {
      if (p->assoc == ASSOC_LEFT) {
        while (nopstack && (p->level > op_se->level)){
          op_se = pop_opstack();
          a2 = pop_numstack();
          if (op_se->unary) {
            r = op_se->eval(a2,0);
          } else {
            a1 = pop_numstack();
            r = op_se->eval(a1, a2);
          }
          push_numstack(r);
        }
        push_opstack(p);
      } else {
        while (nopstack && (p->level >= op_se->level)){
          op_se = pop_opstack();
          a2 = pop_numstack();
          if (op_se->unary) {
            r = op_se->eval(a2,0);
          } else {
            a1 = pop_numstack();
            r = op_se->eval(a1, a2);
          }
          push_numstack(r);
        }
        push_opstack(p);
      }
    }
    buf += strlen(p->operator);
  }
  
  buf = skip_white(buf);

  if (!isendofarg(*buf))
    return eval_expr(buf, &buf, value, op_e, reg);
    
  DBG(printf("1: nopstack = %d, nnumstack = %d\n", nopstack, nnumstack));
  while (nopstack) {
    op_se = pop_opstack();
    a2 = pop_numstack();
    if (op_se->unary) {
      r = op_se->eval(a2,0);
    } else {
      a1 = pop_numstack();
      r = op_se->eval(a1,a2);
    }
    push_numstack(r);
  }
  
  if (nnumstack) {
    *value = pop_numstack();
    DBG(printf("Result = %d\n", *value));
    if (outptr)
      *outptr = buf;
    DBG(printf("2: nopstack = %d, nnumstack = %d\n", nopstack, nnumstack));
  }
  
  return OK;
}

/*
 * Evaluate the address section to see what addressing mode
 * it has.
 */
int evaluate_address(char *buf, struct address_mode *mode)
{
  int error = OK;
  
  buf = skip_white(buf);
  /* First check for immediate addressing mode */
  if (*buf == '#') {
    mode->mode = MODE_IMMEDIATE;
    buf++;
    error = eval_expr(buf, &buf, &mode->value, 0, NULL);
    /* Check for errors */
    if (error)
      goto exit;
    if ((mode->value > 255) || (mode->value < 0))
      error = ASM_ADDR_IMMEDIATE_TO_BIG;
  /* Now check for accumulator */
  } else if ((*buf == 'A' || *buf == 'a') && isendofarg(*(buf+1))) {
    mode->mode = MODE_ACCUMULATOR;
    buf++;
    buf = skip_white(buf);
    if (!isendofline(*buf))
      error = ASM_UNEXPECTED_CHARACTER;
  /* Check for indirect mode */
  } else if (*buf = '(') {
    int reg;
    printf ("indirect mode '%s'\n", buf);
    error = eval_expr(buf, &buf, &mode->value, 0, &reg);
    if (error)
      return error;
    if (reg == BUILT_IN_X) {
      mode->mode = MODE_INDIRECT_IX;
    } else if (reg == BUILT_IN_Y) {
      mode->mode = MODE_INDIRECT_IY;
    } else {
      error = ASM_INVALID_ADDRESSING_MODE;
    }
  }
exit:
    return error;
}
