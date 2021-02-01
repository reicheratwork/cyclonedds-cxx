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

#include <string.h>

#include "idlcxx/backend.h"
#include "idlcxx/backendCpp11Utils.h"
#include "idl/export.h"

static idl_retcode_t
test_keyless(
  const idl_pstate_t* pstate,
  const bool revisit,
  const idl_path_t* path,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)revisit;
  (void)path;

  const idl_member_t* member_node = (const idl_member_t*)node;
  bool* keyless = (bool*)user_data;

  if (idl_mask(member_node) & IDL_KEY) *keyless = false;
  return IDL_RETCODE_OK;
}

static idl_retcode_t
generate_traits(
  const idl_pstate_t* pstate,
  const bool revisit,
  const idl_path_t* path,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)revisit;
  (void)path;

  idl_backend_ctx ctx = (idl_backend_ctx)user_data;
  const idl_struct_t* struct_node = (const idl_struct_t*)node;
  char *struct_name = get_cpp11_fully_scoped_name(node);
  bool is_keyless = true;

  idl_visitor_t visitor;
  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = IDL_MEMBER;
  visitor.accept[IDL_ACCEPT_MEMBER] = &test_keyless;
  if (pstate->sources)
  {
    visitor.sources = malloc(sizeof(char*) * 2);
    visitor.sources[0] = pstate->sources->path->name;
    visitor.sources[1] = NULL;
  }

  idl_retcode_t result = IDL_RETCODE_OK;
  if ((result = idl_visit(pstate, struct_node->members, &visitor, &is_keyless)))
    goto fail;

  idl_file_out_printf(ctx, "template <>\n");
  idl_file_out_printf(ctx, "class TopicTraits<%s>\n", struct_name);
  idl_file_out_printf(ctx, "{\n");
  idl_file_out_printf(ctx, "public:\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "static ::org::eclipse::cyclonedds::topic::DataRepresentationId_t getDataRepresentationId()\n");
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "return ::org::eclipse::cyclonedds::topic::OSPL_REPRESENTATION;\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
  idl_file_out_printf(ctx, "static ::std::vector<uint8_t> getMetaData()\n");
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "return ::std::vector<uint8_t>();\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
  idl_file_out_printf(ctx, "static ::std::vector<uint8_t> getTypeHash()\n");
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "return ::std::vector<uint8_t>();\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
  idl_file_out_printf(ctx, "static ::std::vector<uint8_t> getExtentions()\n");
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "return ::std::vector<uint8_t>();\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
  idl_file_out_printf(ctx, "static bool isKeyless()\n");
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "return %s;\n", is_keyless ? "true" : "false");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
  idl_file_out_printf(ctx, "static const char *getTypeName()\n");
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "return \"%s\";\n", &struct_name[2] /* Skip preceeding "::" according to convention. */);
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
  idl_file_out_printf(ctx, "static ddsi_sertopic *getSerTopic(const std::string& topic_name)\n");
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "auto *st = new ddscxx_sertopic<%s>(topic_name.c_str());\n", struct_name);
  idl_file_out_printf(ctx, "return static_cast<ddsi_sertopic*>(st);\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
  idl_file_out_printf(ctx, "static size_t getSampleSize()\n");
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "return sizeof(%s);\n", struct_name);
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "};\n");

fail:
  free(visitor.sources);
  free(struct_name);
  return result;
}

static idl_retcode_t
generate_macro_call(
  const idl_pstate_t* pstate,
  const bool revisit,
  const idl_path_t* path,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)revisit;
  (void)path;

  idl_backend_ctx ctx = (idl_backend_ctx)user_data;
  char* struct_name = get_cpp11_fully_scoped_name(node);

  idl_file_out_printf(ctx, "REGISTER_TOPIC_TYPE(%s)\n", struct_name);
  free(struct_name);

  return IDL_RETCODE_OK;
}

idl_retcode_t
idl_backendGenerateTrait(idl_backend_ctx ctx, const idl_pstate_t *parse_tree)
{
  idl_retcode_t result = IDL_RETCODE_OK;

  idl_file_out_printf(ctx, "#include \"org/eclipse/cyclonedds/topic/TopicTraits.hpp\"\n");
  idl_file_out_printf(ctx, "#include \"org/eclipse/cyclonedds/topic/DataRepresentation.hpp\"\n");
  idl_file_out_printf(ctx, "#include \"org/eclipse/cyclonedds/topic/datatopic.hpp\"\n\n");
  idl_file_out_printf(ctx, "namespace org { namespace eclipse { namespace cyclonedds { namespace topic {\n");

  idl_visitor_t visitor;

  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = IDL_STRUCT;
  visitor.accept[IDL_ACCEPT_STRUCT] = &generate_traits;
  if (parse_tree->sources)
  {
    visitor.sources = malloc(sizeof(char*) * 2);
    visitor.sources[0] = parse_tree->sources->path->name;
    visitor.sources[1] = NULL;
  }

  if ((result = idl_visit(parse_tree, parse_tree->root, &visitor, ctx)))
    goto fail;

  idl_file_out_printf(ctx, "}}}}\n\n");

  visitor.accept[IDL_ACCEPT_STRUCT] = &generate_macro_call;

  if ((result = idl_visit(parse_tree, parse_tree->root, &visitor, ctx)))
    goto fail;

  idl_file_out_printf(ctx, "\n");

fail:
  free(visitor.sources);
  return result;
}

