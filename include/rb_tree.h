#ifndef RB_TREE_H
#define RB_TREE_H

#include <stddef.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0

typedef uint8_t rb_color_t;

#define RB_RED   ((rb_color_t)0)
#define RB_BLACK ((rb_color_t)1)

struct rb_node {
    struct rb_node *parent;          /* offset 0,  size 8 */
    struct rb_node *l_child;         /* offset 8,  size 8 */
    struct rb_node *r_child;         /* offset 16, size 8 */
    uint64_t        key;             /* offset 24, size 8 */
    rb_color_t      color;           /* offset 32, size 1 */
    uint8_t         _pad[31];        /* offset 33, size 31 -> total 64 */
};

#define RB_NODE_DEFINE(name, value_size) \
    struct rb_node_##name {              \
        struct rb_node node;             \
        char           value[value_size];\
    }

typedef struct rb_tree {
    struct rb_node *root;
    struct rb_node  nil;
} rb_tree_t;

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

int rb_tree_init(struct rb_tree *tree);
int rb_tree_validate(const struct rb_tree *tree);
int rb_tree_insert(struct rb_tree *tree, struct rb_node *node);
int rb_tree_delete(struct rb_tree *tree, struct rb_node *node);
struct rb_node *rb_tree_search(const struct rb_tree *tree, uint64_t key);

#endif