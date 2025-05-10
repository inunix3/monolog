#pragma once

#include "hashmap.h"
#include "vector.h"

#include <stdbool.h>

typedef enum TypeId {
    TYPE_ERROR,
    TYPE_INT,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_LIST,
    TYPE_OPTION,
    TYPE_NIL
} TypeId;

typedef struct Type {
    TypeId id;
    char *name;

    union {
        struct {
            struct Type *type;
        } opt_type;

        struct {
            struct Type *type;
        } list_type;
    };
} Type;

const char *type_name(const Type *type);

static inline bool type_equal(const Type *self, const Type *type) {
    return self->name == type->name;
}

bool type_can_implicitly_convert(const Type *self, const Type *type);

typedef struct TypeSystem {
    /* Used for type interning.
     *
     * If we had only basic types like int, string and
     * void, they could be stored statically. But since there are also
     * parametrized types like list<T> and T?, they have to be allocated. So it
     * will be simpler to store everything here. */
    HashMap types; /* HashMap<char *, Type *> */
    Vector type_names; /* Vector<char *> */

    Type *builtin_int;
    Type *builtin_string;
    Type *builtin_void;
    Type *opt_type;
    Type *error_type;
    Type *nil_type;
} TypeSystem;

bool type_system_init(TypeSystem *self);
void type_system_deinit(TypeSystem *self);
char *type_system_name(TypeSystem *self, const Type *type);
Type *type_system_register(TypeSystem *self, const Type *type);
Type *type_system_get(TypeSystem *self, const char *name);
