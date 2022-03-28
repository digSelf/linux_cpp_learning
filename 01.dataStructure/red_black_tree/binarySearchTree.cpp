/**
 * @file binarySearchTree.cpp
 * @author digSelf (coding@algo.ac.cn)
 * @brief 二叉搜索树的简单实现
 * @version 0.1
 * @date 2022-03-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <cstdio>
#include <vector>

using std::vector;

// 基础数据结构在定义和实现的时候，一定要将数据结构和业务的数据分离开，即：业务和数据结构的算法分离
// 放到二叉搜索树来说，就是树的实现和树要保存的用户的数据之间做到分离，不能强绑定。即：树的节点的定义与key-value的定义分开
#define BSTREE_ENTRY(name, type)                                               \
    struct name {                                                              \
        struct type* left;                                                     \
        struct type* right;                                                    \
    }

// 假设用户的key的类型是int
typedef int USER_KEY_TYPE;

typedef struct bstree_node {
    BSTREE_ENTRY(, bstree_node) entry;

    // 用户数据部分，主要以key作为查找
    USER_KEY_TYPE key;

    // 这里开始定义用户的其他数据部分 ...
} t_bstree_node;

// 两个结构体，第二个结构体只保存二叉搜索树的根节点 
typedef struct bstree {
    t_bstree_node *root;
} t_bstree; 

// 创建二叉搜索树的节点
t_bstree_node *create_bstree_node(USER_KEY_TYPE key) {
    t_bstree_node *node = new t_bstree_node;
    if (nullptr == node) {
        return nullptr;
    }

    node->key = key;
    node->entry.left = node->entry.right = nullptr;
    return node;
}

// 向指定的二叉搜索树中插入节点
int insert_node_to_bstree(t_bstree *tree, USER_KEY_TYPE key) {
    if (nullptr == tree) 
        return -1;

    // 如果当前为空树，则直接创建的节点为树根节点
    if (nullptr == tree->root) {
        t_bstree_node *node = create_bstree_node(key);
        if (nullptr == node) {
            return -2;
        }

        tree->root = node;
        return 0;
    }    
    
    // 否则遍历到合适的位置插入该节点
    t_bstree_node *cursor = tree->root;
    t_bstree_node *cursor_parent = nullptr;
    while (cursor) {
        cursor_parent = cursor;
        
        if (key < cursor->key) {
            cursor = cursor->entry.left;
        } else if (key > cursor->key) {
            cursor = cursor->entry.right;
        } else {
            // 根据不同的场景，对于插入时相等情况的处理不同，可以update，也可以直接丢弃处理
            return 1;
        }
    }

    t_bstree_node *node = create_bstree_node(key);
    if (nullptr == node) {
        return -2;
    }

    if (key < cursor_parent->key) {
        cursor_parent->entry.left = node;
    } else {
        cursor_parent->entry.right = node;
    }

    return 0;
}

// 中序遍历
int inorder_traversal(t_bstree_node *root, vector<int>& result) {
    if (nullptr == root)
        return -1;
    
    inorder_traversal(root->entry.left, result);
    result.emplace_back(root->key);
    inorder_traversal(root->entry.right, result);

    return 0;
}

int main() {
    int nums[] = {12,31,24,5,12,5,34,9,2985,324,5,69,8};
    int len = sizeof(nums) / sizeof(int);

    t_bstree tree = {0};
    for (int i = 0; i < len; ++i) {
        insert_node_to_bstree(&tree, nums[i]);
    }

    vector<int> result;
    inorder_traversal(tree.root, result);
    for (auto key : result) {
        std::printf("%d ", key);
    }
    std::printf("\n");

    return 0;
}