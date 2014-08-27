#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "symbols.h"
#include "errors.h"

extern int PC;
/******************************************************************************
 *                       Support functions
 *****************************************************************************/
/*
 * Skip white space characters
 */
char *skip_white(char *buf)
{
  while (isspace(*buf))
    buf++;

  return buf;
}

/*
 * Skip to next white space character
 */
char *skiptowhite(char *buf)
{
  while (!isspace(*buf))
    buf++;

  return buf;  
}

/*
 * Find the end of a line/command/argument
 * Currently NL, CF and ; is considered to be the end of a line.
 */
char isendofline(char c)
{
  switch (c) {
    case '\0':
    case '\n':
    case '\r':
    case ';':
      return 1;
      
    default:
      return 0;
  }
  return 0;
}

/*
 * Convert next phrase to upper case
 */
void strntoupper(char* result, char *buf, int n)
{
  buf = skip_white(buf);
  while (!isspace(*buf) && *buf != ';' && n--)
    *result++ = toupper(*buf++);
  *result = '\0';
}

/*
 * Find the end of a line/command/argument
 */
char isendofarg(char c)
{
  switch (c) {
    case '\0':
    case '\n':
    case '\r':
    case ';':
    case '\t':
    case ' ':
    case ')':
    case ',':
      return 1;
      
    default:
      return 0;
  }
  return 0;
}

/*
 * get clean argument
 */
char *getarg(char *result, char* buf)
{
  /* First make sure we are on the first character in the argument */
  buf = skip_white(buf);

  /* Now copy the entire argument to the supplied buffer */
  while (!isendofarg(*buf))
    *result++ = *buf++;

  /* Terminate the result */
  *result = '\0';
  return buf;
}

/*
 * Make sure it is a valid character for a label
 */
int isvalidlabel(int c)
{
  if (isalnum(c) || c == '_')
    return 1;
  else
    return 0;
}

/*
 * Read and store the current label.
 * The function returns a pointer to the created symbol entry
 * so that the caller can modify the value property if needed.
 */
struct symbol_entry *read_and_store_label(char *buf)
{
  char label[256];
  char *labptr;
  struct symbol_entry *se;
  int i=1;

  label[0] = *buf++;

  while (isvalidlabel(*buf) && (i < 255))
    label[i++] = *buf++;
  label[i] = '\0';
  
  printf ("LAB: '%s'\n", label);
  
  /* Create storage for the new label */
  labptr = (char *)malloc(i+1);
  if (!labptr) {
    printf("Could not allocate necessary memory, terminating !\n");
    exit(1);
  }
  strcpy(labptr, label);
  /* Create a new symbol entry */
  se = sym_new_symbol(labptr);
  if (!se)
    return NULL;
  se->symbol_name = labptr;
  se->name_length = i;
  se->value = PC;

  /* In case the label name was longer than max num characters */
  while (!isspace(*buf++));
  
  return se;
}

/*
 * Read a value from the buffer
 */
int getvalue(char *buf)
{
  char arg[256];
  int base = 10;
  int pos = 0;
  int value;
  
  getarg(arg, buf);
  /* The first character is significant for determining
     the type of data here */
  if (isdigit(arg[0]) || arg[0] == '$') {
    /* it's a number, read it */
    if (arg[pos] == '$') {
      base = 16;
      pos++;
    }
    value = strtoul(&arg[pos], NULL, base);
  }
  return value;
}

/*
 * Converts a power of 2 addressing mode to its decimal
 * representation. Only use this function for addressing modes.
 */
int mode2dec(unsigned int val)
{
  int i = 0;
  
  /* Sanity check */
  if (!val) return -1;
  
  while (val) {
    i++;
    val >>= 1;
  }
  
  return (i - 2);
}
