#ifndef RB_TREE_H
#define RB_TREE_H

#include <stddef.h>

#define TRUE 1
#define FALSE 0

typedef enum {
    RB_RED = 0,
    RB_BLACK = 1
} rb_color_t;

struct rb_node {
    struct rb_node *parent;
    struct rb_node *l_child;
    struct rb_node *r_child;
    rb_color_t color;
    int key;
    char value[];
};

typedef struct rb_tree {
    struct rb_node *root;
    struct rb_node *nil;
} rb_tree_t;

int rb_tree_init(struct rb_tree *tree);
int rb_tree_validate(const struct rb_tree *tree);
int rb_tree_insert(struct rb_tree *tree, int key, const void *value, size_t value_size);
int rb_tree_delete(struct rb_tree *tree, int key);
struct rb_node *rb_tree_search_by_key(const struct rb_tree *tree, int key);
struct rb_node *rb_tree_search_by_value(const struct rb_tree *tree, const void *value,
                                        size_t value_size);

#endif