#include <limits.h>
#include "test.h"
#include "rb_tree.h"

static int insert_key(struct rb_tree *t, int key)
{
    static int dummy_value = 0;
    return rb_tree_insert(t, key, &dummy_value, sizeof(dummy_value));
}

static int rb_test_check_subtree(struct rb_node *node, struct rb_node *nil,
                                 int min_excl, int max_excl, int *errors)
{
    if (node == nil) {
        return 1;
    }
    if (node->key <= min_excl || node->key >= max_excl) {
        printf("  invariant: BST order violated at key=%d\n", node->key);
        (*errors)++;
    }
    if (node->color == RB_RED) {
        if (node->l_child != nil && node->l_child->color == RB_RED) {
            printf("  invariant: red %d has red left child %d\n",
                   node->key, node->l_child->key);
            (*errors)++;
        }
        if (node->r_child != nil && node->r_child->color == RB_RED) {
            printf("  invariant: red %d has red right child %d\n",
                   node->key, node->r_child->key);
            (*errors)++;
        }
    }
    int lh = rb_test_check_subtree(node->l_child, nil, min_excl, node->key, errors);
    int rh = rb_test_check_subtree(node->r_child, nil, node->key, max_excl, errors);
    if (lh != rh) {
        printf("  invariant: black-height mismatch at key=%d (left=%d, right=%d)\n",
               node->key, lh, rh);
        (*errors)++;
    }
    return lh + (node->color == RB_BLACK ? 1 : 0);
}

static int rb_test_validate(struct rb_tree *tree)
{
    if (tree == NULL || tree->nil == NULL) {
        printf("  invariant: tree not initialized (nil is NULL)\n");
        return 0;
    }
    if (tree->nil->color != RB_BLACK) {
        printf("  invariant: sentinel (nil) is not black\n");
        return 0;
    }
    if (tree->root == tree->nil) {
        return 1;
    }
    if (tree->root->color != RB_BLACK) {
        printf("  invariant: root is not black\n");
        return 0;
    }
    int errors = 0;
    rb_test_check_subtree(tree->root, tree->nil, INT_MIN, INT_MAX, &errors);
    return errors == 0;
}

static void test_init_returns_zero(void)
{
    struct rb_tree tree = {0};
    TEST_ASSERT_EQ(rb_tree_init(&tree), 0);
}

static void test_validate_empty_tree(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    TEST_ASSERT(rb_tree_validate(&tree));
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_search_by_key_on_empty(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 42));
}

static void test_search_by_value_on_empty(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    int needle = 0;
    TEST_ASSERT_NULL(rb_tree_search_by_value(&tree, &needle, sizeof(needle)));
}

static void test_delete_on_empty_keeps_validity(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    rb_tree_delete(&tree, 42);
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_insert_single(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    TEST_ASSERT_EQ(insert_key(&tree, 1), 0);
    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, 1));
}

static void test_insert_two_ascending(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 1);
    insert_key(&tree, 2);
    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, 1));
    TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, 2));
}

static void test_insert_two_descending(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 2);
    insert_key(&tree, 1);
    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, 1));
    TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, 2));
}

static void test_insert_three_balanced(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 2);
    insert_key(&tree, 1);
    insert_key(&tree, 3);
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_insert_uncle_red_recolor(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 10);
    insert_key(&tree, 5);
    insert_key(&tree, 15);
    insert_key(&tree, 3);
    TEST_ASSERT(rb_test_validate(&tree));
    static const int keys[] = {10, 5, 15, 3};
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, keys[i]));
    }
}

static void test_insert_zigzag_left_right(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 10);
    insert_key(&tree, 5);
    insert_key(&tree, 7);
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_insert_zigzag_right_left(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 5);
    insert_key(&tree, 10);
    insert_key(&tree, 7);
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_insert_sequential_ascending(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    for (int k = 1; k <= 50; k++) {
        insert_key(&tree, k);
        TEST_ASSERT(rb_test_validate(&tree));
    }
    for (int k = 1; k <= 50; k++) {
        TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, k));
    }
}

static void test_insert_sequential_descending(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    for (int k = 50; k >= 1; k--) {
        insert_key(&tree, k);
        TEST_ASSERT(rb_test_validate(&tree));
    }
    for (int k = 1; k <= 50; k++) {
        TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, k));
    }
}

static void test_insert_balanced_pattern(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    static const int keys[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45, 55, 65, 75, 85};
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        insert_key(&tree, keys[i]);
        TEST_ASSERT(rb_test_validate(&tree));
    }
}

static void test_insert_duplicate_key_keeps_validity(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 5);
    insert_key(&tree, 5);
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_search_missing_after_inserts(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 10);
    insert_key(&tree, 20);
    insert_key(&tree, 30);
    TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 999));
    TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 0));
    TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 15));
}

static void test_delete_only_node(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 1);
    rb_tree_delete(&tree, 1);
    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 1));
}

static void test_delete_then_reinsert(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 1);
    rb_tree_delete(&tree, 1);
    TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 1));
    insert_key(&tree, 1);
    TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, 1));
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_delete_root_with_two_children(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 50);
    insert_key(&tree, 30);
    insert_key(&tree, 70);
    rb_tree_delete(&tree, 50);
    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 50));
    TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, 30));
    TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, 70));
}

static void test_delete_all_inserted_in_order(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    for (int k = 1; k <= 30; k++) {
        insert_key(&tree, k);
    }
    for (int k = 1; k <= 30; k++) {
        rb_tree_delete(&tree, k);
        TEST_ASSERT(rb_test_validate(&tree));
    }
    TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 1));
    TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 30));
}

static void test_delete_all_inserted_reverse(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    for (int k = 1; k <= 30; k++) {
        insert_key(&tree, k);
    }
    for (int k = 30; k >= 1; k--) {
        rb_tree_delete(&tree, k);
        TEST_ASSERT(rb_test_validate(&tree));
    }
}

static void test_delete_in_pseudo_random_order(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    static const int keys[]  = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45, 55, 65, 75, 85};
    static const int order[] = {30, 70, 50, 40, 60, 20, 80, 10, 25, 35, 45, 55, 65, 75, 85};
    size_t n = sizeof(keys) / sizeof(keys[0]);
    for (size_t i = 0; i < n; i++) {
        insert_key(&tree, keys[i]);
    }
    for (size_t i = 0; i < n; i++) {
        rb_tree_delete(&tree, order[i]);
        TEST_ASSERT(rb_test_validate(&tree));
    }
}

static void test_delete_nonexistent_keeps_validity(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    insert_key(&tree, 5);
    rb_tree_delete(&tree, 999);
    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, 5));
}

static void test_alternate_insert_delete_same_key(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    for (int i = 0; i < 100; i++) {
        insert_key(&tree, 42);
        TEST_ASSERT(rb_test_validate(&tree));
        TEST_ASSERT_NOT_NULL(rb_tree_search_by_key(&tree, 42));
        rb_tree_delete(&tree, 42);
        TEST_ASSERT(rb_test_validate(&tree));
        TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 42));
    }
}

static void test_alternating_insert_delete_growing(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    for (int i = 1; i <= 50; i++) {
        insert_key(&tree, i);
        TEST_ASSERT(rb_test_validate(&tree));
        if (i > 1) {
            rb_tree_delete(&tree, i - 1);
            TEST_ASSERT(rb_test_validate(&tree));
        }
    }
}

static void test_stress_500_mixed_operations(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    for (int i = 1; i <= 500; i++) {
        int k = (i * 7919) % 10000 + 1;
        insert_key(&tree, k);
    }
    TEST_ASSERT(rb_test_validate(&tree));
    for (int i = 1; i <= 500; i += 2) {
        int k = (i * 7919) % 10000 + 1;
        rb_tree_delete(&tree, k);
    }
    TEST_ASSERT(rb_test_validate(&tree));
    for (int i = 0; i < 250; i++) {
        int k = (i * 6997) % 10000 + 5000;
        insert_key(&tree, k);
    }
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_pathological_skewed_then_drain(void)
{
    struct rb_tree tree = {0};
    rb_tree_init(&tree);
    for (int k = 1; k <= 100; k++) {
        insert_key(&tree, k);
        TEST_ASSERT(rb_test_validate(&tree));
    }
    for (int k = 1; k <= 100; k++) {
        rb_tree_delete(&tree, k);
        TEST_ASSERT(rb_test_validate(&tree));
    }
    TEST_ASSERT_NULL(rb_tree_search_by_key(&tree, 50));
}

int main(void)
{
    TEST_RUN(test_init_returns_zero);
    TEST_RUN(test_validate_empty_tree);
    TEST_RUN(test_search_by_key_on_empty);
    TEST_RUN(test_search_by_value_on_empty);
    TEST_RUN(test_delete_on_empty_keeps_validity);
    TEST_RUN(test_insert_single);
    TEST_RUN(test_insert_two_ascending);
    TEST_RUN(test_insert_two_descending);
    TEST_RUN(test_insert_three_balanced);
    TEST_RUN(test_insert_uncle_red_recolor);
    TEST_RUN(test_insert_zigzag_left_right);
    TEST_RUN(test_insert_zigzag_right_left);
    TEST_RUN(test_insert_sequential_ascending);
    TEST_RUN(test_insert_sequential_descending);
    TEST_RUN(test_insert_balanced_pattern);
    TEST_RUN(test_insert_duplicate_key_keeps_validity);
    TEST_RUN(test_search_missing_after_inserts);
    TEST_RUN(test_delete_only_node);
    TEST_RUN(test_delete_then_reinsert);
    TEST_RUN(test_delete_root_with_two_children);
    TEST_RUN(test_delete_all_inserted_in_order);
    TEST_RUN(test_delete_all_inserted_reverse);
    TEST_RUN(test_delete_in_pseudo_random_order);
    TEST_RUN(test_delete_nonexistent_keeps_validity);
    TEST_RUN(test_alternate_insert_delete_same_key);
    TEST_RUN(test_alternating_insert_delete_growing);
    TEST_RUN(test_stress_500_mixed_operations);
    TEST_RUN(test_pathological_skewed_then_drain);
    return TEST_SUMMARY();
}