/*
 * Handles outputting the binary code
 */
struct output_descriptor {
  int length;
  unsigned char *data;
};

int output(struct output_descriptor *od);
