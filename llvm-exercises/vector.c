// file: vector_example.c
#include <math.h>
#include <stddef.h>

static inline double dot(const double *a, const double *b, size_t n) {
  double s = 0.0;
  for (size_t i = 0; i < n; i++)
    s += a[i] * b[i];
  return s;
}

static inline double norm(const double *a, size_t n) {
  return sqrt(dot(a, a, n));
}

double cosine_similarity(const double *a, const double *b, size_t n) {
  double d = dot(a, b, n);
  double na = norm(a, n);
  double nb = norm(b, n);
  return d / (na * nb);
}

int main(int argc, char **argv) {
  double x[3] = {1, 2, 3};
  double y[3] = {4, 5, 6};
  return (int)(cosine_similarity(x, y, 3) * 1000);
}
