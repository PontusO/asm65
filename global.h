/*
 * Global variables
 */
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

enum addressing_modes {
  MODE_UNUSED      = 0x0001,  // 0
  MODE_ACCUMULATOR = 0x0002,  // 1   0
  MODE_ABSOLUTE    = 0x0004,  // 2   1
  MODE_ABSOLUTE_IX = 0x0008,  // 3   2
  MODE_ABSOLUTE_IY = 0x0010,  // 4   3
  MODE_IMMEDIATE   = 0x0020,  // 5   4
  MODE_IMPLIED     = 0x0040,  // 6   5
  MODE_INDIRECT    = 0x0080,  // 7   6
  MODE_INDIRECT_IX = 0x0100,  // 8   7
  MODE_INDIRECT_IY = 0x0200,  // 9   8
  MODE_RELATIVE    = 0x0400,  // 10  9
  MODE_ZEROPAGE    = 0x0800,  // 11  10
  MODE_ZEROPAGE_IX = 0x1000,  // 12  11
  MODE_ZEROPAGE_IY = 0x2000,  // 13  12
};

extern int cpu;
extern int PC;
extern int line;
extern int pass;

#endif // __GLOBAL_H__
