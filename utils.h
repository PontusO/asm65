
#ifndef __UTILS_H__
#define __UTILS_H__

char *skip_white(char *buf);
char *skiptowhite(char *buf);
char isendofline(char c);
void strntoupper(char* result, char *buf, int n);
char *getarg(char *result, char* buf);
int isvalidlabel(int c);
struct symbol_entry *read_and_store_label(char *buf);
int getvalue(char *buf);
int mode2dec(unsigned int val);

#endif // __UTILS_H__