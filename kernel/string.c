/*
 * string.c - Freestanding C runtime primitives.
 */

#include "string.h"

void *memcpy(void *dest, const void *src, size_t n)
{
  if (!dest || !src || dest == src || n == 0)
    return dest;

  uint8_t *d = (uint8_t *)dest;
  const uint8_t *s = (const uint8_t *)src;
  for (size_t i = 0; i < n; i++)
  {
    d[i] = s[i];
  }
  return dest;
}

void *memset(void *s, int c, size_t n)
{
  if (!s || n == 0)
    return s;

  uint8_t *p = (uint8_t *)s;
  uint8_t val = (uint8_t)c;
  for (size_t i = 0; i < n; i++)
  {
    p[i] = val;
  }
  return s;
}

/*
 * Safely handles overlapping source and destination memory buffers.
 */
void *memmove(void *dest, const void *src, size_t n)
{
  if (!dest || !src || dest == src || n == 0)
    return dest;

  uint8_t *d = (uint8_t *)dest;
  const uint8_t *s = (const uint8_t *)src;
  if (d < s)
  {
    for (size_t i = 0; i < n; i++)
    {
      d[i] = s[i];
    }
  }
  else if (d > s)
  {
    for (size_t i = n; i > 0; i--)
    {
      d[i - 1] = s[i - 1];
    }
  }
  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  if (!s1 && !s2)
    return 0;
  if (!s1)
    return -1;
  if (!s2)
    return 1;
  if (s1 == s2 || n == 0)
    return 0;

  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;
  for (size_t i = 0; i < n; i++)
  {
    if (p1[i] != p2[i])
    {
      return p1[i] - p2[i];
    }
  }
  return 0;
}

size_t strlen(const char *s)
{
  if (!s)
    return 0;

  size_t len = 0;
  while (s[len])
  {
    len++;
  }
  return len;
}
