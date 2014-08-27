
#ifndef __EXPR_H__
#define __EXPR_H__

struct address_mode {
  int mode;
  int value;
};

int eval_expr(char *buf, char **outptr, int *value, int op_expected, int *reg);
int evaluate_address(char *buf, struct address_mode *mode);

#endif // __EXPR_H__