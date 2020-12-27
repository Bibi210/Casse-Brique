/*!\file plateau.c
 * \author Farès BELHADJ
 */
#include <stdlib.h>
#include <assert.h>

unsigned int * plateau(int w, int h) {
  int i, j;
  int * lab;
  lab = malloc(w * h * sizeof *lab);
  assert(lab);
  for(i = 0; i < h; ++i)
    for(j = 0; j< w; ++j)
      if((i == 0 || i == h-1) || (j == 0 || j == w - 1))
	lab[i * w + j] = -1;
      else
	lab[i * w + j] = 0;
  return (unsigned int *)lab;
}
