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

int rb_tree_validate(const struct rb_tree *tree, rb_cmp_fn cmp)
{
    return TRUE;
}

int rb_tree_insert(struct rb_tree *tree, struct rb_node *node, rb_cmp_fn cmp)
{
    return 0;
}

int rb_tree_delete(struct rb_tree *tree, struct rb_node *node)
{
    return 0;
}

struct rb_node *rb_tree_search(const struct rb_tree *tree, const struct rb_node *probe,
                               rb_cmp_fn cmp)
{
    return NULL;
}