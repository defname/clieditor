#include "tableiterator.h"

TableIterator TableIterator_Begin(const Table *table) {
    return (TableIterator){
        .table = table,
        .index = -1,
        .current = NULL
    };
}

bool TableIterator_Next(TableIterator *it) {
    if (!it || !it->table) {
        return false;
    }
    for (size_t i = (size_t)(it->index + 1); i < it->table->capacity; i++) {
        TableSlot *slot = &it->table->slots[i];
        if (slot->state == TABLE_SLOT_USED) {
            it->index = i;
            it->current = slot;
            return true;
        }
    }
    return false;
}
