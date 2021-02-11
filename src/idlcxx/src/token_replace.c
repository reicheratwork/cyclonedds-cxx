#include "idlcxx/token_replace.h"
#include "idl/string.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

#define _TOKEN_START '{'
#define _TOKEN_END '}'
#define _ESCAPED '%'

static const char* template_token_names[] = { "{TYPE}", "{BOUND}", NULL };
static const char* template_token_formats[] = { "s", PRIu64, NULL };

static size_t istok(const char* str, size_t len, const char** token_names)
{
  for (size_t num = 0; token_names[num]; num++) {
    if (strncmp(token_names[num], str, len) == 0)
      return num + 1;
  }
  return 0;
}

int idl_replace_tokens_default(char** output, const char* input)
{
  return idl_replace_tokens_with_indices(output, input, template_token_names, template_token_formats);
}

int idl_replace_tokens_with_indices(char **output, const char* input, const char** token_names, const char** token_flags)
{
  char buf[64], * fmt;
  size_t len = 0;
  size_t num, src, tok, dest;

#define FMT "%%%zu$%s"

  src = tok = 0;
  do {
    if (tok) {
      if (input[src] == _TOKEN_END && (num = istok(input + tok, src - tok, token_names))) {
        const char* flag = token_flags[num - 1];
        len += (size_t)idl_snprintf(buf, sizeof(buf), FMT, num, flag);
        tok = 0;
      }
      else if (input[src] == _TOKEN_END || input[src] == '\0') {
        /* unknown token, rewind */
        len += 1;
        src = tok - 1;
        tok = 0;
      }
    }
    else if (input[src] == _TOKEN_START) {
      tok = src + 1;
    }
    else if (input[src]) {
      len += (size_t)(1 + (input[src] == '%')); /* escape % */
    }
  } while (input[src++]);

  if (!(fmt = malloc(len + 1)))
    return -1;

  src = tok = dest = 0;
  do {
    if (tok) {
      if (input[src] == _TOKEN_END && (num = istok(input + tok, src - tok, token_names))) {
        const char* flag = token_flags[num - 1];
        dest += (size_t)idl_snprintf(&fmt[dest], (len - dest) + 1, FMT, num, flag);
        tok = 0;
      }
      else if (input[src] == _TOKEN_END || input[src] == '\0') {
        fmt[dest++] = _TOKEN_START;
        src = tok - 1;
        tok = 0;
      }
    }
    else if (input[src] == _TOKEN_START) {
      tok = src + 1;
    }
    else if (input[src] == '%') {
      fmt[dest++] = '%';
      fmt[dest++] = '%';
    }
    else if (input[src]) {
      fmt[dest++] = input[src];
    }
  } while (input[src++]);
  assert(dest == len);
  fmt[len] = '\0';

#undef FMT

  * output = fmt;
  return (int)len;
}

int idl_replace_indices_with_values(char** output, const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  va_list args_copy;
  va_copy(args_copy, args);
  int wb = idl_vsnprintf(*output,
    0,
    fmt,
    args);
  va_end(args);

  if (wb < 0)
  {
    fprintf(stderr, "a formatting error occurred during format_ostream\n");
    return -1;
  }
  size_t writtenbytes = (size_t)wb+1;

  char* temp = malloc(writtenbytes);
  if (!temp)
  {
    fprintf(stderr, "could not allocate buffer during replacement of token indices with values\n");
    return -2;
  }

  wb = idl_vsnprintf(temp,
    writtenbytes,
    fmt,
    args_copy);
  va_end(args_copy);

  if (wb < 0)
  {
    fprintf(stderr, "a formatting error occurred during format_ostream\n");
    free(temp);
    return -1;
  }

  *output = temp;

  return wb;
}
