/*
 * Result codes for the symbol library
 */
enum symbol_results {
  SYM_OK,
};

/*
 * Symbol entry descriptor
 */
struct symbol_entry;
struct symbol_entry {
  struct symbol_entry *prev;
  struct symbol_entry *next;
  char *symbol_name;
  int name_length;
  int value;
};

struct built_in_symbol {
  char *name;
  int (*getvalue)(void);
};

enum built_in_descriptors {
  BUILT_IN_X = 1,
  BUILT_IN_Y,
};
  
/* Pointer to the first entry in the list */
extern struct symbol_entry *se_first;
/* Pointer to the last entry in the list */
extern struct symbol_entry *se_last;
/* The number of symbols in the list */
extern int num_symbols;

void sym_init(void);
struct symbol_entry *sym_new_symbol(char *buf);
struct symbol_entry *sym_next_symbol(struct symbol_entry *entry);
struct symbol_entry *sym_look_for_symbol(char *buf, char **out_ptr);
int sym_get_symbol_value(char *buf, char **out);
struct built_in_symbol *check_built_in_symbol(char *buf, char **out_ptr);
int sym_clean_up(void);
