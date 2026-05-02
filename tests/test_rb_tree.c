#include <inttypes.h>
#include <stdint.h>
#include "test.h"
#include "rb_tree.h"

struct test_rec {
    uint64_t       id;
    struct rb_node node;
};

static int test_rec_cmp(const struct rb_node *a, const struct rb_node *b)
{
    const struct test_rec *ra = container_of(a, struct test_rec, node);
    const struct test_rec *rb = container_of(b, struct test_rec, node);

    if (ra->id < rb->id) {
        return -1;
    }
    if (ra->id > rb->id) {
        return 1;
    }
    return 0;
}

static uint64_t test_rec_id(const struct rb_node *n)
{
    return container_of(n, struct test_rec, node)->id;
}

static void insert_id(struct rb_tree *tree, struct test_rec *rec, uint64_t id)
{
    rec->id = id;
    rb_tree_insert(tree, &rec->node, test_rec_cmp);
}

static struct rb_node *find_id(struct rb_tree *tree, uint64_t id)
{
    struct test_rec probe = { .id = id, };
    return rb_tree_search(tree, &probe.node, test_rec_cmp);
}

static int rb_test_check_subtree(const struct rb_node *node, const struct rb_node *nil,
                                 uint64_t low_excl, uint64_t high_excl,
                                 int *out_black_height)
{
    if (node == nil) {
        *out_black_height = 1;
        return 0;
    }

    int errors = 0;
    uint64_t key = test_rec_id(node);

    if (key <= low_excl || key >= high_excl) {
        printf("  invariant: BST order violated at key=%" PRIu64 "\n", key);
        errors++;
    }

    if (node->color == RB_RED) {
        if (node->l_child != nil && node->l_child->color == RB_RED) {
            uint64_t lkey = test_rec_id(node->l_child);
            printf("  invariant: red %" PRIu64 " has red left child %" PRIu64 "\n",
                   key, lkey);
            errors++;
        }

        if (node->r_child != nil && node->r_child->color == RB_RED) {
            uint64_t rkey = test_rec_id(node->r_child);
            printf("  invariant: red %" PRIu64 " has red right child %" PRIu64 "\n",
                   key, rkey);
            errors++;
        }
    }

    int lh = 0;
    int rh = 0;
    errors += rb_test_check_subtree(node->l_child, nil, low_excl, key, &lh);
    errors += rb_test_check_subtree(node->r_child, nil, key, high_excl, &rh);

    if (lh != rh) {
        printf("  invariant: black-height mismatch at key=%" PRIu64
               " (left=%d, right=%d)\n", key, lh, rh);
        errors++;
    }

    int self_black = (node->color == RB_BLACK) ? 1 : 0;
    *out_black_height = lh + self_black;
    return errors;
}

static int rb_test_validate(const struct rb_tree *tree)
{
    if (tree == NULL) {
        printf("  invariant: tree is NULL\n");
        return 0;
    }

    if (tree->nil.color != RB_BLACK) {
        printf("  invariant: sentinel not black (init may not have set it up)\n");
        return 0;
    }

    if (tree->root == NULL) {
        printf("  invariant: root is NULL (init may not have set it up)\n");
        return 0;
    }

    if (tree->root == &tree->nil) {
        return 1;
    }

    if (tree->root->color != RB_BLACK) {
        printf("  invariant: root is not black\n");
        return 0;
    }

    int black_height = 0;
    int errors = rb_test_check_subtree(tree->root, &tree->nil, 0, UINT64_MAX,
                                       &black_height);
    return errors == 0;
}

static void test_init_returns_zero(void)
{
    struct rb_tree tree = { 0, };

    TEST_ASSERT_EQ(rb_tree_init(&tree), 0);
}

static void test_validate_empty_tree(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    TEST_ASSERT(rb_tree_validate(&tree, test_rec_cmp));
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_search_on_empty(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    TEST_ASSERT_NULL(find_id(&tree, 42));
}

static void test_delete_on_empty_keeps_validity(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec stub = { .id = 42, };
    rb_tree_delete(&tree, &stub.node);

    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_insert_single(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec rec;
    insert_id(&tree, &rec, 1);

    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NOT_NULL(find_id(&tree, 1));
}

static void test_insert_two_ascending(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec r1, r2;
    insert_id(&tree, &r1, 1);
    insert_id(&tree, &r2, 2);

    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NOT_NULL(find_id(&tree, 1));
    TEST_ASSERT_NOT_NULL(find_id(&tree, 2));
}

static void test_insert_two_descending(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec r1, r2;
    insert_id(&tree, &r1, 2);
    insert_id(&tree, &r2, 1);

    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NOT_NULL(find_id(&tree, 1));
    TEST_ASSERT_NOT_NULL(find_id(&tree, 2));
}

static void test_insert_three_balanced(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec r1, r2, r3;
    insert_id(&tree, &r1, 2);
    insert_id(&tree, &r2, 1);
    insert_id(&tree, &r3, 3);

    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_insert_uncle_red_recolor(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    static const uint64_t keys[] = { 10, 5, 15, 3, };
    size_t n = sizeof(keys) / sizeof(keys[0]);
    struct test_rec recs[4];

    for (size_t i = 0; i < n; i++) {
        insert_id(&tree, &recs[i], keys[i]);
    }

    TEST_ASSERT(rb_test_validate(&tree));

    for (size_t i = 0; i < n; i++) {
        TEST_ASSERT_NOT_NULL(find_id(&tree, keys[i]));
    }
}

static void test_insert_zigzag_left_right(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec r1, r2, r3;
    insert_id(&tree, &r1, 10);
    insert_id(&tree, &r2, 5);
    insert_id(&tree, &r3, 7);

    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_insert_zigzag_right_left(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec r1, r2, r3;
    insert_id(&tree, &r1, 5);
    insert_id(&tree, &r2, 10);
    insert_id(&tree, &r3, 7);

    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_insert_sequential_ascending(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec recs[50];
    for (int k = 0; k < 50; k++) {
        insert_id(&tree, &recs[k], (uint64_t)(k + 1));
        TEST_ASSERT(rb_test_validate(&tree));
    }

    for (int k = 1; k <= 50; k++) {
        TEST_ASSERT_NOT_NULL(find_id(&tree, (uint64_t)k));
    }
}

static void test_insert_sequential_descending(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec recs[50];
    for (int k = 0; k < 50; k++) {
        insert_id(&tree, &recs[k], (uint64_t)(50 - k));
        TEST_ASSERT(rb_test_validate(&tree));
    }

    for (int k = 1; k <= 50; k++) {
        TEST_ASSERT_NOT_NULL(find_id(&tree, (uint64_t)k));
    }
}

static void test_insert_balanced_pattern(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    static const uint64_t keys[] = {
        50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45, 55, 65, 75, 85,
    };
    size_t n = sizeof(keys) / sizeof(keys[0]);
    struct test_rec recs[15];

    for (size_t i = 0; i < n; i++) {
        insert_id(&tree, &recs[i], keys[i]);
        TEST_ASSERT(rb_test_validate(&tree));
    }
}

static void test_insert_duplicate_key_keeps_validity(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec r1, r2;
    insert_id(&tree, &r1, 5);
    insert_id(&tree, &r2, 5);

    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_search_missing_after_inserts(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec r1, r2, r3;
    insert_id(&tree, &r1, 10);
    insert_id(&tree, &r2, 20);
    insert_id(&tree, &r3, 30);

    TEST_ASSERT_NULL(find_id(&tree, 999));
    TEST_ASSERT_NULL(find_id(&tree, 0));
    TEST_ASSERT_NULL(find_id(&tree, 15));
}

static void test_delete_only_node(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec rec;
    insert_id(&tree, &rec, 1);
    rb_tree_delete(&tree, &rec.node);

    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NULL(find_id(&tree, 1));
}

static void test_delete_then_reinsert(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec rec1;
    insert_id(&tree, &rec1, 1);
    rb_tree_delete(&tree, &rec1.node);
    TEST_ASSERT_NULL(find_id(&tree, 1));

    struct test_rec rec2;
    insert_id(&tree, &rec2, 1);
    TEST_ASSERT_NOT_NULL(find_id(&tree, 1));
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_delete_root_with_two_children(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec r1, r2, r3;
    insert_id(&tree, &r1, 50);
    insert_id(&tree, &r2, 30);
    insert_id(&tree, &r3, 70);

    rb_tree_delete(&tree, &r1.node);

    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NULL(find_id(&tree, 50));
    TEST_ASSERT_NOT_NULL(find_id(&tree, 30));
    TEST_ASSERT_NOT_NULL(find_id(&tree, 70));
}

static void test_delete_all_inserted_in_order(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec recs[30];
    for (int k = 0; k < 30; k++) {
        insert_id(&tree, &recs[k], (uint64_t)(k + 1));
    }

    for (int k = 0; k < 30; k++) {
        rb_tree_delete(&tree, &recs[k].node);
        TEST_ASSERT(rb_test_validate(&tree));
    }

    TEST_ASSERT_NULL(find_id(&tree, 1));
    TEST_ASSERT_NULL(find_id(&tree, 30));
}

static void test_delete_all_inserted_reverse(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec recs[30];
    for (int k = 0; k < 30; k++) {
        insert_id(&tree, &recs[k], (uint64_t)(k + 1));
    }

    for (int k = 29; k >= 0; k--) {
        rb_tree_delete(&tree, &recs[k].node);
        TEST_ASSERT(rb_test_validate(&tree));
    }
}

static void test_delete_in_pseudo_random_order(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    static const uint64_t keys[] = {
        50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45, 55, 65, 75, 85,
    };
    static const int del_idx[] = {
        1, 2, 0, 4, 5, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    };
    size_t n = sizeof(keys) / sizeof(keys[0]);
    struct test_rec recs[15];

    for (size_t i = 0; i < n; i++) {
        insert_id(&tree, &recs[i], keys[i]);
    }

    for (size_t i = 0; i < n; i++) {
        rb_tree_delete(&tree, &recs[del_idx[i]].node);
        TEST_ASSERT(rb_test_validate(&tree));
    }
}

static void test_delete_nonexistent_keeps_validity(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec rec;
    insert_id(&tree, &rec, 5);

    struct test_rec stub = { .id = 999, };
    rb_tree_delete(&tree, &stub.node);

    TEST_ASSERT(rb_test_validate(&tree));
    TEST_ASSERT_NOT_NULL(find_id(&tree, 5));
}

static void test_alternate_insert_delete_same_key(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    for (int i = 0; i < 100; i++) {
        struct test_rec rec = { .id = 42, };
        rb_tree_insert(&tree, &rec.node, test_rec_cmp);
        TEST_ASSERT(rb_test_validate(&tree));
        TEST_ASSERT_NOT_NULL(find_id(&tree, 42));

        rb_tree_delete(&tree, &rec.node);
        TEST_ASSERT(rb_test_validate(&tree));
        TEST_ASSERT_NULL(find_id(&tree, 42));
    }
}

static void test_alternating_insert_delete_growing(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    struct test_rec recs[50];
    for (int i = 0; i < 50; i++) {
        insert_id(&tree, &recs[i], (uint64_t)(i + 1));
        TEST_ASSERT(rb_test_validate(&tree));

        if (i > 0) {
            rb_tree_delete(&tree, &recs[i - 1].node);
            TEST_ASSERT(rb_test_validate(&tree));
        }
    }
}

static void test_stress_500_mixed_operations(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    static struct test_rec recs[500];
    for (int i = 0; i < 500; i++) {
        int hashed = (i + 1) * 7919;
        int wrapped = hashed % 10000 + 1;
        insert_id(&tree, &recs[i], (uint64_t)wrapped);
    }
    TEST_ASSERT(rb_test_validate(&tree));

    for (int i = 0; i < 500; i += 2) {
        rb_tree_delete(&tree, &recs[i].node);
    }
    TEST_ASSERT(rb_test_validate(&tree));

    static struct test_rec extras[250];
    for (int i = 0; i < 250; i++) {
        int hashed = i * 6997;
        int wrapped = hashed % 10000 + 5000;
        insert_id(&tree, &extras[i], (uint64_t)wrapped);
    }
    TEST_ASSERT(rb_test_validate(&tree));
}

static void test_pathological_skewed_then_drain(void)
{
    struct rb_tree tree = { 0, };
    rb_tree_init(&tree);

    static struct test_rec recs[100];
    for (int k = 0; k < 100; k++) {
        insert_id(&tree, &recs[k], (uint64_t)(k + 1));
        TEST_ASSERT(rb_test_validate(&tree));
    }

    for (int k = 0; k < 100; k++) {
        rb_tree_delete(&tree, &recs[k].node);
        TEST_ASSERT(rb_test_validate(&tree));
    }

    TEST_ASSERT_NULL(find_id(&tree, 50));
}

int main(void)
{
    TEST_RUN(test_init_returns_zero);
    TEST_RUN(test_validate_empty_tree);
    TEST_RUN(test_search_on_empty);
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