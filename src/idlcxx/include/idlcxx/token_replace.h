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
#ifndef IDL_TOKEN_REPLACE_H
#define IDL_TOKEN_REPLACE_H

int idl_replace_tokens_with_indices(char** output, const char* input, const char** token_names, const char **token_flags);

int idl_replace_tokens_default(char** output, const char* input);

int idl_replace_indices_with_values(char** output, const char* fmt, ...);

#endif