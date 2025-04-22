#include <monolog/type.h>

const char *type_to_string(TypeId type) {
    switch (type) {
    case TYPE_UNKNOWN:
        return "unknown";
    case TYPE_INT:
        return "int";
    case TYPE_STRING:
        return "string";
    case TYPE_VOID:
        return "void";
    }
}
