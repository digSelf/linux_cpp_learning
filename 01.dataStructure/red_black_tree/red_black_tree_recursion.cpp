/**
 * @file red_black_tree_recursion.cpp
 * @author digSelf (coding@algo.ac.cn)
 * @brief 红黑树的递归版本
 * @version 0.1
 * @date 2022-04-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <cstdio>
#include <vector>

using std::vector;

typedef int USER_KEY_TYPE;

#define RBTREE_CLR_RED 0
#define RBTREE_CLR_BLK 1
#define RBTREE_CLR_DBL 2

typedef struct rbtree_node {
    USER_KEY_TYPE key;
    int color;
    struct rbtree_node *left;
    struct rbtree_node *right;    
} t_rbtree_node;

typedef struct rbtree {
    t_rbtree_node *root;
} t_rbtree;

t_rbtree_node __nil_node;

#define nil_node (&__nil_node)

#define SAFE_DELETE_NODE(node) {if (node) { delete node; node = nullptr; }}

__attribute__((constructor))
void init_nil_node() {
     __nil_node.key = 0;
     __nil_node.color = RBTREE_CLR_BLK;
     __nil_node.left = __nil_node.right = nil_node;
}

t_rbtree_node *rbtree_create_node(USER_KEY_TYPE key) {
    t_rbtree_node *node = new t_rbtree_node;
    if (node == nullptr) {
        return nullptr;
    }

    node->key = key;
    node->color = RBTREE_CLR_RED;
    node->left = node->right = nil_node;

    return node;
}

// 销毁以指定节点为根节点的子树
void __rbtree_destroy(t_rbtree_node *root) {
    if (root == nil_node) 
        return;
    
    __rbtree_destroy(root->left);
    __rbtree_destroy(root->right);
    SAFE_DELETE_NODE(root);
}

// 销毁整棵树
void rbtree_destroy(t_rbtree *tree) {
    if (tree == nullptr) 
        return;

    __rbtree_destroy(tree->root);
}

int has_red_child_node(t_rbtree_node *node) {
    if (nullptr == node)
        return -1;
    
    return node->left->color == RBTREE_CLR_RED || node->right->color == RBTREE_CLR_RED;
}

// 以node为旋转轴心，进行左旋，并返回旋转过后的新的根节点
t_rbtree_node *rbtree_left_rotate(t_rbtree_node *up) {
    // root指向旋转后的根节点的地址
    t_rbtree_node *down = up->right;
    up->right = down->left;
    down->left = up;

    return down;
}

// 以up为轴心，进行右旋，并返回右旋过后的新的根节点
t_rbtree_node *rbtree_right_rotate(t_rbtree_node *up) {
    t_rbtree_node *down = up->left;
    up->left = down->right;
    down->right = up;

    return down;
}

// 给定待调整的根节点，返回调整后的根节点的位置
#define RBTREE_INSERT_CLT_NONE  0
#define RBTREE_INSERT_CLT_LEFT  1
#define RBTREE_INSERT_CLT_RIGHT 2
t_rbtree_node *rbtree_insert_maintian(t_rbtree_node *root) {
    // 当当前节点没有红 色孩子的时候不需要进行调整
    if (!has_red_child_node(root))
        return root; 

    // 当当前节点的两个孩子均为红色的时候，为了方便，不做额外判断，直接改为`红黑黑`的形状，即：红色节点上浮
    // 这么做并不会影响红黑树的平衡，因此可以统一这么修改
    if (root->left->color == RBTREE_CLR_RED && root->right->color == RBTREE_CLR_RED) {
        root->color = RBTREE_CLR_RED;
        root->left->color = root->right->color = RBTREE_CLR_BLK;

        return root;
    }

    // 判断是否有冲突。此时：儿子节点只可能有一个节点是红色的；那么在这个红色节点下又有一个孙子节点是红色的话就产生冲突了
    int has_red_conflict = RBTREE_INSERT_CLT_NONE;
    if (root->left->color == RBTREE_CLR_RED && has_red_child_node(root->left)) {
        has_red_conflict = RBTREE_INSERT_CLT_LEFT;
    } else if (root->right->color == RBTREE_CLR_RED && has_red_child_node(root->right)) {
        has_red_conflict = RBTREE_INSERT_CLT_RIGHT;
    }

    if (!has_red_conflict) // 没有冲突，就不用进行调整了
        return root;

    if (has_red_conflict == RBTREE_INSERT_CLT_LEFT) {
        // LL / LR
        // 先判断是否需要小左旋
        if (root->left->right->color == RBTREE_CLR_RED) {
            root->left = rbtree_left_rotate(root->left);
        }
        // LL是一定要一个大右旋的
        root = rbtree_right_rotate(root);
    } else if (has_red_conflict == RBTREE_INSERT_CLT_RIGHT) {
        // RL / RR
        if (root->right->left->color == RBTREE_CLR_RED) {
            root->right = rbtree_right_rotate(root->right);
        }
        root = rbtree_left_rotate(root);
    }
    
    root->color = RBTREE_CLR_RED;
    root->left->color = root->right->color = RBTREE_CLR_BLK;
    return root;
}

// 插入新节点，返回根节点
t_rbtree_node *__rbtree_insert(t_rbtree_node *root, USER_KEY_TYPE key) {
    if (root == nil_node)
        return rbtree_create_node(key);

    // 插入的点有重复，不进行重复插入
    if (root->key == key) 
        return root;

    if (key < root->key) {
        root->left = __rbtree_insert(root->left, key);
    } else {
        root->right = __rbtree_insert(root->right, key);
    }

    // 插入调整应该发生在回溯的过程中
    return rbtree_insert_maintian(root);
}

// 向树中插入新节点
void rbtree_insert(t_rbtree *tree, USER_KEY_TYPE key) {
    if (tree == nullptr) 
        return;
    
    tree->root = __rbtree_insert(tree->root, key);
    tree->root->color = RBTREE_CLR_BLK;
}

// 中序遍历
void __inorder_traversal(t_rbtree_node *root, vector<int>& result) {
    if (root == nil_node) {
        return;
    }

    __inorder_traversal(root->left, result);
    result.emplace_back(root->key);
    __inorder_traversal(root->right, result);
}
int inorder_traversal(t_rbtree *tree, vector<int>& result) {
    if (nullptr == tree)
        return -1;
    
    __inorder_traversal(tree->root, result);
    return 0;
}

int main() {
    // 因为有重复的，所以只保存一遍，共10个不重复元素
    int nums[] = {12,31,24,5,12,5,34,9,2985,324,5,69,8};
    int len = sizeof(nums) / sizeof(int);

    t_rbtree tree = {0};
    tree.root = nil_node;

    for (int i = 0; i < len; ++i) {
        rbtree_insert(&tree, nums[i]);
    }

    vector<int> result;
    inorder_traversal(&tree, result);
    for (auto key : result) {
        std::printf("%d ", key);
    }
    std::printf("\n");

    return 0;
}