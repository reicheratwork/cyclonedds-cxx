#include "idlcxx/processor_options.h"
#include <inttypes.h>

const char* sequence_template = "idl_sequence<%1$s, %2$"PRIu64">";
const char* sequence_include = "<org/eclipse/cyclonedds/topic/cdr_dummys.hpp>";
const char* array_template = "idl_array<%1$s, %2$"PRIu64">";
const char* array_include = "<org/eclipse/cyclonedds/topic/cdr_dummys.hpp>";
const char* bounded_sequence_template = "idl_sequence<%1$s, %2$"PRIu64">";
const char* bounded_sequence_include = "<org/eclipse/cyclonedds/topic/cdr_dummys.hpp>";
const char* string_template = "idl_string<%1$"PRIu64">";
const char* string_include = "<org/eclipse/cyclonedds/topic/cdr_dummys.hpp>";
const char* bounded_string_template = "idl_string<%1$"PRIu64">";
const char* bounded_string_include = "<org/eclipse/cyclonedds/topic/cdr_dummys.hpp>";
const char* union_template = "std::variant";
const char* union_getter_template = "std::get";
const char* union_include = "<variant>";

