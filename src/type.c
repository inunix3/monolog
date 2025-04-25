#include <monolog/type.h>

const char *type_name(const Type *type) {
    /* TODO: int? should return option<int>, same applies to arrays */

    switch (type->id) {
    case TYPE_INT:
        return "integer";
    case TYPE_STRING:
        return "string";
    case TYPE_VOID:
        return "void";
    case TYPE_OPTION:
        return "option";
    case TYPE_ARRAY:
        return "array";
    case TYPE_ERROR:
        return "<error>";
    }
}
