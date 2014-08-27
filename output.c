/*
 * Handles outputting the binary code
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
#include "output.h"

#ifdef DEBUG_EXPR
#define DBG(x) x
#else
#define DBG(x)
#endif

int send_to_file(struct output_descriptor *od)
{
  return OK;
}

int output(struct output_descriptor *od)
{
  int i;
  int error = OK;
  
  printf ("PC %04x: ", PC);
  for(i=0;i<od->length;i++) {
    printf(" %02x", od->data[i]);
  }
  printf("\n");
  
  error = send_to_file(od);
  /* Update the address pointer */
  PC += od->length;
  
  return error;
}