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
#include <inttypes.h>
#include "idlcxx/backend.h"

// replace the definitions below (CPP11_...) with your own custom classes and includes.

//sequence templates
#define SEQUENCE_TEMPLATE(type, bound)                 "idl_bounded_sequence<%s, %"PRIu64">", type, bound

// array templates
#define ARRAY_TEMPLATE(element, const_expr)     "idl_array<%s, %s>", element, const_expr

// string templates
#define STRING_TEMPLATE(bound)                  "idl_bounded_string<%"PRIu64">", bound

#define CPP11_UNION_TEMPLATE                          "std::variant"
#define CPP11_UNION_GETTER_TEMPLATE                   "std::get"
#define CPP11_UNION_INCLUDE                           "<variant>"

char*
get_cpp11_name(const char* name);

char *
get_cpp11_type(const idl_node_t *node);

char *
get_cpp11_fully_scoped_name(const idl_node_t *node);

char *
get_default_value(idl_backend_ctx ctx, const idl_node_t *node);

char *
get_cpp11_const_value(const idl_constval_t *literal);

char *
get_cpp11_literal_value(const idl_literal_t *literal);

