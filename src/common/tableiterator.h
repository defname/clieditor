#ifndef TABLEITERATOR_H
#define TABLEITERATOR_H

#include <sys/types.h>
#include "table.h"

typedef struct _TableIterator {
    const Table *table;
    ssize_t index;
    const TableSlot *current;
} TableIterator;

TableIterator TableIterator_Begin(const Table *table);
bool TableIterator_Next(TableIterator *it);

#endif