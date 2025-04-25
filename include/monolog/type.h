#pragma once

typedef enum TypeId {
    TYPE_ERROR,
    TYPE_INT,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_ARRAY,
    TYPE_OPTION,
} TypeId;

typedef struct Type {
    TypeId id;

    union {
        struct {
            struct Type *type;
        } opt_type;

        struct {
            struct Type *type;
        } array_type;
    };
} Type;

const char *type_name(const Type *type);
