#include "table.h"

Entry *table_entry(Table *table, bb key) {
  return &table->entry[key & table->mask];
};

void table_free(Table *table) { free(table->entry); };

void table_prefetch(Table *table, bb key) {
  __builtin_prefetch(table_entry(table, key));
};

void table_clear(Table *table) { memset(table, 0, sizeof(Table)); }

int table_alloc(Table *table, int bits) {
  table_clear(table);
  table->size = 1 << bits;
  table->mask = table->size - 1;
  table->entry = calloc(table->size, sizeof(Entry));
  if (table->entry == NULL) {
    err("table_alloc(): failed to allocate transposition table entries");
    return 0;
  }
  return 1;
}

Move table_get_move(Table *table, bb key) {
  Entry *entry = table_entry(table, key);
  if (entry->key == key) {
    return entry->move;
  }

  return 0;
}

void table_set_move(Table *table, bb key, int depth, Move move) {
  Entry *entry = table_entry(table, key);
  if (entry->depth <= depth) {
    entry->key = key;
    entry->depth = depth;
    entry->move = move;
  }
}

void table_set(Table *table, bb key, int depth, int value, int flag) {
  Entry *entry = table_entry(table, key);
  if (entry->depth <= depth) {
    entry->key = key;
    entry->depth = depth;
    entry->score = value;
    entry->flag = flag;
  }
}

int table_get(Table *table, bb key, int depth, int alpha, int beta,
              int *value) {
  Entry *entry = table_entry(table, key);
  int flag = 0;
  if (entry->key == key) {
    if (entry->depth >= depth) {
      if (entry->flag == EXACT) {
        *value = entry->score;
        flag = 1;
      }
      if ((entry->flag == ALPHA) && (entry->score <= alpha)) {
        *value = alpha;
        flag = 1;
      }
      if ((entry->flag == BETA) && (entry->score >= beta)) {
        *value = beta;
        flag = 1;
      }
    }
  }
  return flag;
}

int pawn_table_alloc(PawnTable *table, int bits) {
  table->size = 1 << bits;
  table->mask = table->size - 1;
  table->entry = calloc(table->size, sizeof(PawnEntry));

  if (table->entry == NULL) {
    err("pawn_table_alloc(): failed to allocate pawn transposition table "
        "entries");
    return 0;
  }
  return 1;
}

INLINE PawnEntry *pawn_table_entry(PawnTable *table, bb key) {
  return &table->entry[key & table->mask];
}

void pawn_table_set(PawnTable *table, bb key, int value) {
  PawnEntry *entry = pawn_table_entry(table, key);
  entry->key = key;
  entry->value = value;
}

int pawn_table_get(PawnTable *tabke, bb key) {
  PawnEntry *entry = pawn_table_entry(tabke, key);
  if (entry->key == key)
    return entry->value;
  return 0;
}

void pawn_table_free(PawnTable *table) { return free(table->entry); }