#include "style.h"


bool Style_Cmp(const Style *a, const Style *b) {
    return a->fg == b->fg && a->bg == b->bg && a->attributes == b->attributes;
}