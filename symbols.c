#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "symbols.h"
#include "global.h"
#include "utils.h"

//#define DEBUG_SYM
#ifdef DEBUG_SYM
#define DBG(x) x
#else
#define DBG(x)
#endif

/* Pointer to the first entry in the list */
struct symbol_entry *se_first;
/* Pointer to the last entry in the list */
struct symbol_entry *se_last;
/* The number of symbols in the list */
int num_symbols;

int bis_getx(void);
int bis_gety(void);
int bis_getpc(void);

struct built_in_symbol bis[] = {
  { "X", bis_getx   },
  { "x", bis_getx   },
  { "Y", bis_gety   },
  { "y", bis_gety   },
  { "*", bis_getpc  },
  { "PC", bis_getpc },
  { "pc", bis_getpc },
  { NULL, NULL },
};

/******************************************************************************
 *                       Built in symbol return value getters
 *****************************************************************************/
int bis_getx(void)
{
  return BUILT_IN_X;
}

int bis_gety(void)
{
  return BUILT_IN_Y;
}

int bis_getpc(void)
{
  return PC;
}

/******************************************************************************
 *                       Symbol management
 *****************************************************************************/
/*
 * initialize the symbol management
 */
void sym_init(void)
{
  se_first = NULL;
  se_last = NULL;
  num_symbols = 0;
}

/*
 * Add a new symbol at the end of the symbol table
 */
struct symbol_entry *sym_new_symbol(char *buf)
{
  struct symbol_entry *se;

  /* First we should make sure the symbol doesn't already exist */
  if (sym_look_for_symbol(buf, NULL))
    return NULL;
  
  /* Now, create a new entry */
  se = (struct symbol_entry *)malloc(sizeof (struct symbol_entry));
  
  if (!se) {
    printf ("Could not allocate necessary memory, exiting !\n");
    exit(1);
  }
  
  /* First entry in table need special treatment */
  if (!num_symbols) {
    se_first = se;
    se_last = se;
    se->prev = NULL;
    se->next = NULL;
  } else {
    /* Change the previously last entry to point to this one */
    se_last->next = se;
    /* The new entry prev pointer shall point to the previously last one */
    se->prev = se_last;
    /* The new entry is the current last */
    se_last = se;
    /* There is no next in the new last entry */
    se->next = NULL;
  }
  num_symbols++;
}

/*
 * Get next entry in the list
 */
struct symbol_entry *sym_next_symbol(struct symbol_entry *entry)
{
  if (entry->next)
    return entry->next;
  else
    return NULL;
}

/*
 * Read a symbol name and zero terminate it.
 * Only uses the first 255 characters of the symbol name.
 */
char *sym_read_name(char *result, char *buf)
{
  int i = 1;
  
  *result++ = *buf++;
  while (isvalidlabel(*buf) && (i++ < 255))
    *result++ = *buf++;
  *result = '\0';
  
  return buf;
}

/*
 * Look for a symbol in the symbol list.
 */
struct symbol_entry *sym_look_for_symbol(char *buf, char **outptr)
{
  struct symbol_entry *se = se_first;
  struct built_in_symbol *lbis = &bis[0];
  int cnt = num_symbols;
  char symbol[256];
  
  /* get next section */
  getarg(symbol, buf);
  while (cnt--) {
    if (!strncmp(symbol, se->symbol_name, strlen(symbol))) {
      if (outptr)
        *outptr += se->name_length;
      return se;
    }
    se = se->next;
  }
  return NULL;
}

/*
 * Clean up after using the symbol table
 */
int sym_clean_up(void)
{
  struct symbol_entry *se = se_first;
  struct symbol_entry *next;
  int cnt = num_symbols;
  
  while (cnt--) {
    /* Get the pointer to the next entry before freeing */
    next = se->next;
    free(se->symbol_name);
    free(se);
    se = next;
  }
  return SYM_OK;
}

struct built_in_symbol *check_built_in_symbol(char *buf, char **outptr)
{
  struct built_in_symbol *lbis = &bis[0];
  char symbol[256];

  DBG(printf("i"));
  if (outptr)
    *outptr = getarg(symbol, buf);
  else
    getarg(symbol, buf);
    
  while (*lbis->name) {
    if (!strncmp(symbol, lbis->name, strlen(lbis->name))) {
      DBG(printf("o(%s)\n", lbis->name));
      return lbis;
    }
    lbis++;
  }
  DBG(printf("x\n"));
  return NULL;
}
