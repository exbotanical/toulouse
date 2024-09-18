#ifndef K_MATH_H
#define K_MATH_H

#define min(a, b)      ((a < b) ? a : b)
#define max(a, b)      ((a > b) ? a : b)

#define div_down(x, y) ((x) / (y))
#define div_up(x, y) \
  ((((uint32_t)(x)) == 0) ? (0) : (((((uint32_t)(x)) - 1) / ((uint32_t)(y))) + 1))

#endif /* K_MATH_H */
