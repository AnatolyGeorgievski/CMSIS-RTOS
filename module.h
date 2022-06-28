#ifndef MODULE_H_INCLUDED
#define MODULE_H_INCLUDED

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef struct _ModuleInfo ModuleInfo;
struct _ModuleInfo
{
    char *name;
    void* data;
    void* (*init)(void* data);
    void (*scan)(void* data);
};

#include "segment.h"
//__attribute__((used, section("MOD")))

#define MODULE(mod_name, mod_init, mod_scan) ModuleInfo mod_name  SEGMENT_ATTR(MOD) \
    = { .name = #mod_name, \
        .data = NULL, \
        .init = mod_init, \
        .scan = mod_scan }

#define XML_SCHEMA_ATTR  __attribute__((used, section("XML_TAGS")))

#endif // MODULE_H_INCLUDED
