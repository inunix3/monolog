#pragma once

typedef enum TypeId {
    TYPE_UNKNOWN,
    TYPE_INT,
    TYPE_STRING,
    TYPE_VOID,
} TypeId;

const char *type_to_string(TypeId type);
