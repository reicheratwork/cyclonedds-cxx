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

#include "CUnit/Theory.h"

#include "idlcxx/token_replace.h"

#include <stdlib.h>

static const char* _tokens[] = {
  "tok1",
  "tok2",
  "tok3",
  "unused_tok",
  "another_unused_tok",
  NULL
};

static const char* _flags[] = {
  "s",
  "d",
  "u",
  "x",
  "s",
  NULL
};

void tok_to_ind()
{
  const char *string_in = "abc {tok2} def {tok3} {tok1} ghi",
    *test_out = "abc %2$d def %3$u %1$s ghi";
  char* string_out = NULL;
  CU_ASSERT(idl_replace_tokens_with_indices(&string_out, string_in, _tokens, _flags) > 0);
  CU_ASSERT_STRING_EQUAL(test_out,string_out);

  if (string_out)
    free(string_out);
}

void ind_to_val()
{
  const char *string_in = "abc %2$s def %3$d %1$c ghi",
    *test_out = "abc string_1 def 654 c ghi";
  char* string_out = NULL;
  CU_ASSERT(idl_replace_indices_with_values(&string_out, string_in, 'c', "string_1", 654) > 0);

  CU_ASSERT_STRING_EQUAL(test_out,string_out);

  if (string_out)
    free(string_out);
}

CU_Test(token_replace, tokens_to_indices)
{
  tok_to_ind();
}

CU_Test(token_replace, indices_to_values)
{
  ind_to_val();
}
