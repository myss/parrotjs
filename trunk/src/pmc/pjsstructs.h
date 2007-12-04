
#ifndef __PJSSTRUCTS_H__
#define __PJSSTRUCTS_H__

#include "parrot/parrot.h"

#define PJS_HASH_ENTRY__READ_ONLY   1
#define PJS_HASH_ENTRY__DONT_ENUM   2
#define PJS_HASH_ENTRY__DONT_DELETE 4
#define PJS_HASH_ENTRY__INTERNAL    8

typedef struct pjs_prop_entry_struct {
    INTVAL flags;
} pjs_prop_entry_struct;

#endif
