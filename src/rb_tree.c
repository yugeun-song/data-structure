#include "rb_tree.h"

int rb_tree_init(struct rb_tree *tree)
{
    return 0;
}

static void rb_tree_l_rotate(struct rb_tree *tree, struct rb_node *x)
{
}

static void rb_tree_r_rotate(struct rb_tree *tree, struct rb_node *x)
{
}

int rb_tree_validate(const struct rb_tree *tree)
{
    return TRUE;
}

int rb_tree_insert(struct rb_tree *tree, int key, const void *value, size_t value_size)
{
    return 0;
}

int rb_tree_delete(struct rb_tree *tree, int key)
{
    return 0;
}

struct rb_node *rb_tree_search_by_key(const struct rb_tree *tree, int key)
{
    return NULL;
}

struct rb_node *rb_tree_search_by_value(const struct rb_tree *tree, const void *value,
                                        size_t value_size)
{
    return NULL;
}