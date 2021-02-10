/*
 * Copyright(c) 2020 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include "idlcxx/processor_options.h"
#include "idl/string.h"
#include "idlcxx/backendCpp11Utils.h"
#include "idlcxx/token_replace.h"

#ifdef _WIN32
#pragma warning(disable : 4996)
#endif

/* Specify a list of all C++11 keywords */
static const char* cpp11_keywords[] =
{
  /* QAC EXPECT 5007; Bypass qactools error */
  "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand",
  "bitor", "bool", "break", "case", "catch", "char", "char16_t",
  "char32_t", "class", "compl", "concept", "const", "constexpr",
  "const_cast", "continue", "decltype", "default", "delete",
  "do", "double", "dynamic_cast", "else", "enum", "explicit",
  "export", "extern", "false", "float", "for", "friend",
  "goto", "if", "inline", "int", "long", "mutable",
  "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or",
  "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
  "requires", "return", "short", "signed", "sizeof", "static", "static_assert",
  "static_cast", "struct", "switch", "template", "this", "thread_local", "throw",
  "true", "try", "typedef", "typeid", "typename", "union", "unsigned",
  "using", "virtual", "void", "volatile", "wchar_t", "while",
  "xor", "xor_eq",
  "int16_t", "int32_t", "int64_t",
  "uint8_t", "uint16_t", "uint32_t", "uint64_t",
};

char*
get_cpp11_name(const char* name)
{
  char* cpp11Name;
  size_t i;

  /* search through the C++ keyword list */
  for (i = 0; i < sizeof(cpp11_keywords) / sizeof(char*); i++) {
    if (strcmp(cpp11_keywords[i], name) == 0) {
      /* If a keyword matches the specified identifier, prepend _cxx_ */
      /* QAC EXPECT 5007; will not use wrapper */
      size_t cpp11NameLen = strlen(name) + 5 + 1;
      cpp11Name = malloc(cpp11NameLen);
      snprintf(cpp11Name, cpp11NameLen, "_cxx_%s", name);
      return cpp11Name;
    }
  }
  /* No match with a keyword is found, thus return the identifier itself */
  cpp11Name = idl_strdup(name);
  return cpp11Name;
}

static char *
get_cpp11_base_type(const idl_node_t *node)
{
  switch (idl_mask(node) & IDL_BASE_TYPE_MASK)
  {
  case IDL_CHAR:
    return idl_strdup("char");
  case IDL_WCHAR:
    return idl_strdup("wchar");
  case IDL_BOOL:
    return idl_strdup("bool");
  case IDL_INT8:
    return idl_strdup("int8_t");
  case IDL_UINT8:
  case IDL_OCTET:
    return idl_strdup("uint8_t");
  case IDL_SHORT:
  case IDL_INT16:
    return idl_strdup("int16_t");
  case IDL_USHORT:
  case IDL_UINT16:
    return idl_strdup("uint16_t");
  case IDL_LONG:
  case IDL_INT32:
    return idl_strdup("int32_t");
  case IDL_ULONG:
  case IDL_UINT32:
    return idl_strdup("uint32_t");
  case IDL_LLONG:
  case IDL_INT64:
    return idl_strdup("int64_t");
  case IDL_ULLONG:
  case IDL_UINT64:
    return idl_strdup("uint64_t");
  case IDL_FLOAT:
    return idl_strdup("float");
  case IDL_DOUBLE:
    return idl_strdup("double");
  case IDL_LDOUBLE:
    return idl_strdup("long double");
  default:
    assert(0);
    break;
  }
  return NULL;
}

static char *
get_cpp11_templ_type(const idl_node_t *node)
{
  char* cpp11Type = NULL;

  switch (idl_mask(node) & IDL_TEMPL_TYPE_MASK)
  {
  case IDL_SEQUENCE:
    {
      char *typeval = get_cpp11_type(((const idl_sequence_t*)node)->type_spec);

      uint64_t bound = ((const idl_sequence_t*)node)->maximum;
      idl_replace_indices_with_values(&cpp11Type, bound ? bounded_sequence_template : sequence_template, typeval, bound);

      free(typeval);
    }
    break;
  case IDL_STRING:
    {
      uint64_t bound = ((const idl_string_t*)node)->maximum;
      idl_replace_indices_with_values(&cpp11Type, bound ? bounded_string_template : string_template, bound);
    }
    break;
  case IDL_WSTRING:
    //currently not supported
    assert(0);
    break;
  case IDL_FIXED_PT:
    //currently not supported
    assert(0);
    break;
  default:
    assert(0);
    break;
  }

  return cpp11Type;
}

char *
get_cpp11_type(const idl_node_t *node)
{
  assert(idl_mask(node) & (IDL_BASE_TYPE|IDL_TEMPL_TYPE|IDL_CONSTR_TYPE|IDL_DECLARATOR));
  if (idl_is_base_type(node))
    return get_cpp11_base_type(node);
  else if (idl_is_templ_type(node))
    return get_cpp11_templ_type(node);
  else
    return get_cpp11_fully_scoped_name(node);
}

char *
get_cpp11_fully_scoped_name(const idl_node_t *node)
{
  char* scoped_enumerator = idl_strdup("");
  char* scope_name = NULL;
  const idl_node_t *current_node = node;

  while (current_node)
  {
    switch (idl_mask(current_node) & (IDL_ENUMERATOR | IDL_UNION | IDL_ENUM | IDL_MODULE | IDL_STRUCT | IDL_TYPEDEF))
    {
    case IDL_ENUMERATOR:
    case IDL_UNION:
    case IDL_ENUM:
    case IDL_MODULE:
    case IDL_STRUCT:
      scope_name = get_cpp11_name(idl_identifier(current_node));
      break;
    case IDL_TYPEDEF:
      scope_name = get_cpp11_name(idl_identifier(((const idl_typedef_t*)current_node)->declarators));
      break;
    }

    if (scope_name)
    {
      char* temp = NULL;
      if (idl_asprintf(&temp, "::%s%s", scope_name, scoped_enumerator) == -1)
        goto fail;
      free(scope_name);
      free(scoped_enumerator);
      scoped_enumerator = temp;
    }

    current_node = current_node->parent;
  }
  return scoped_enumerator;

fail:
  free(scope_name);
  free(scoped_enumerator);

  return NULL;
}

char *
get_default_value(idl_backend_ctx ctx, const idl_node_t *node)
{
  const idl_node_t *unwinded_node = idl_unalias(node, 0x0);
  (void)ctx;

  if (idl_is_enum(unwinded_node))
    return get_cpp11_fully_scoped_name((idl_node_t*)((idl_enum_t*)unwinded_node)->enumerators);

  switch (idl_mask(unwinded_node) & IDL_BASE_TYPE_MASK)
  {
  case IDL_BOOL:
    return idl_strdup("false");
  case IDL_CHAR:
  case IDL_WCHAR:
  case IDL_OCTET:
    return idl_strdup("0");
  case IDL_FLOAT:
    return idl_strdup("0.0f");
  case IDL_DOUBLE:
  case IDL_LDOUBLE:
    return idl_strdup("0.0");
  case IDL_INT8:
  case IDL_UINT8:
  case IDL_INT16:
  case IDL_UINT16:
  case IDL_SHORT:
  case IDL_USHORT:
  case IDL_INT32:
  case IDL_UINT32:
  case IDL_LONG:
  case IDL_ULONG:
  case IDL_INT64:
  case IDL_UINT64:
  case IDL_LLONG:
  case IDL_ULLONG:
    return idl_strdup("0");
  default:
    return NULL;
  }
}

static char *
get_cpp11_base_type_const_value(const idl_literal_t* constval)
{
  int cnt = -1;
  char *str = NULL;

  switch (idl_mask(&constval->node) & IDL_BASE_TYPE_MASK)
  {
  case IDL_BOOL:
    return idl_strdup(constval->value.bln ? "true" : "false");
  case IDL_INT8:
    cnt = idl_asprintf(&str, "%" PRId8, constval->value.int8);
    break;
  case IDL_UINT8:
  case IDL_OCTET:
    cnt = idl_asprintf(&str, "%" PRIu8, constval->value.uint8);
    break;
  case IDL_INT16:
  case IDL_SHORT:
    cnt = idl_asprintf(&str, "%" PRId16, constval->value.int16);
    break;
  case IDL_UINT16:
  case IDL_USHORT:
    cnt = idl_asprintf(&str, "%" PRIu16, constval->value.uint16);
    break;
  case IDL_INT32:
  case IDL_LONG:
    cnt = idl_asprintf(&str, "%" PRId32, constval->value.int32);
    break;
  case IDL_UINT32:
  case IDL_ULONG:
    cnt = idl_asprintf(&str, "%" PRIu32, constval->value.uint32);
    break;
  case IDL_INT64:
  case IDL_LLONG:
    cnt = idl_asprintf(&str, "%" PRId64, constval->value.int64);
    break;
  case IDL_UINT64:
  case IDL_ULLONG:
    cnt = idl_asprintf(&str, "%" PRIu64, constval->value.uint64);
    break;
  case IDL_FLOAT:
    cnt = idl_asprintf(&str, "%.6f", constval->value.flt);
    break;
  case IDL_DOUBLE:
    cnt = idl_asprintf(&str, "%f", constval->value.dbl);
    break;
  case IDL_LDOUBLE:
    cnt = idl_asprintf(&str, "%Lf", constval->value.ldbl);
    break;
  case IDL_CHAR:
    cnt = idl_asprintf(&str, "\'%c\'", constval->value.chr);
    break;
  case IDL_STRING:
    cnt = idl_asprintf(&str, "\"%s\"", constval->value.str);
    break;
  default:
    assert(0);
    break;
  }

  return cnt >= 0 ? str : NULL;
}

static char *
get_cpp11_templ_type_const_value(const idl_literal_t *constval)
{
  char *str;
  size_t len;

  if (!idl_is_string(constval))
    return NULL;
  assert(constval->value.str);
  len = strlen(constval->value.str);
  if ((str = malloc(len + 2 /* quotes */ + 1)) == NULL)
    return NULL;
  str[0] = '"';
  memcpy(str + 1, constval->value.str, len);
  str[1 + len] = '"';
  str[1 + len + 1] = '\0';
  return str;
}

char *
get_cpp11_const_value(const idl_literal_t *constval)
{
  static const idl_mask_t mask = IDL_BASE_TYPE | IDL_TEMPL_TYPE | IDL_ENUMERATOR;

  switch (idl_mask(&constval->node) & mask) {
  case IDL_BASE_TYPE:
    return get_cpp11_base_type_const_value(constval);
  case IDL_TEMPL_TYPE:
    return get_cpp11_templ_type_const_value(constval);
  case IDL_ENUMERATOR:
    return get_cpp11_fully_scoped_name((const idl_node_t *)constval);
  default:
    assert(0);
    break;
  }
  return NULL;
}

uint64_t
array_entries(const idl_const_expr_t* ce)
{
  if (0 == (idl_mask((const idl_node_t*)ce) & IDL_LITERAL))
    return 0;

  const idl_literal_t* var = (const idl_literal_t*)ce;
  switch (idl_mask(var) & IDL_BASE_TYPE_MASK)
  {
  case IDL_UINT8:
    return var->value.uint8;
    break;
  case IDL_USHORT:
  case IDL_UINT16:
    return var->value.uint16;
    break;
  case IDL_ULONG:
  case IDL_UINT32:
    return var->value.uint32;
    break;
  case IDL_ULLONG:
  case IDL_UINT64:
    return var->value.uint64;
    break;
  default:
    return 0;
  }
}

uint64_t
generate_array_expression(char** type_name, const idl_const_expr_t* ce)
{
  uint64_t ret = 0;
  const idl_const_expr_t* tmp = ce;
  /*go forward so that ce ends at the last entry in the list*/
  while (tmp)
  {
    ce = tmp;
    tmp = ((idl_node_t*)tmp)->next;
  }

  /*go backwards through the list, so that the last entries in the list are the innermost arrays*/
  while (ce)
  {
    uint64_t entries = array_entries(ce);
    if (entries)
    {
      char* temp = NULL;
      idl_replace_indices_with_values(&temp, array_template, *type_name, entries);
      free(*type_name);
      *type_name = temp;

      if (ret)
        ret *= entries;
      else
        ret = entries;
    }
    ce = ((idl_node_t*)ce)->previous;
  }

  return ret;
}

void
resolve_namespace(idl_node_t* node, char** up)
{
  if (!node)
    return;

  if (!*up)
    *up = idl_strdup("");

  if (idl_is_module(node))
  {
    idl_module_t* mod = (idl_module_t*)node;
    char* cppname = get_cpp11_name(idl_identifier(mod));
    assert(cppname);
    char* temp = NULL;
    idl_asprintf(&temp, "%s::%s", cppname, *up);
    free(*up);
    *up = temp;
    free(cppname);
  }

  resolve_namespace(node->parent, up);
}
