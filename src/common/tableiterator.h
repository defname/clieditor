#ifndef TABLEITERATOR_H
#define TABLEITERATOR_H

#include <unistd.h>
#include "table.h"

typedef struct _TableIterator {
    const Table *table;
    ssize_t index;
    TableSlot *current;
} TableIterator;

TableIterator TableIterator_Begin(const Table *table);
bool TableIterator_Next(TableIterator *it);

#endif