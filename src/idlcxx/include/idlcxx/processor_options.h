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
#ifndef IDL_PROCESSOR_OPTIONS_H
#define IDL_PROCESSOR_OPTIONS_H

#include <stddef.h>

//options list
const char* sequence_template;
const char* sequence_include;
const char* array_template;
const char* array_include;
const char* bounded_sequence_template;
const char* bounded_sequence_include;
const char* string_template;
const char* string_include;
const char* bounded_string_template;
const char* bounded_string_include;
const char* union_template;
const char* union_getter_template;
const char* union_include;

#endif
