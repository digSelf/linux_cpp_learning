/**
 * @file rbTree.cpp
 * @author digSelf (coding@algo.ac.cn)
 * @brief 红黑树
 * @version 0.1
 * @date 2022-03-29
 *
 * @copyright Copyright (c) 2022
 * 红黑树的应用：Linux进程调度CFS，Nginx Timer事件管理，Epoll事件块的管理
 */

#include <cstdio>
#include <vector>

using std::vector;

// 红黑树的4条性质：
// 1. 所有的节点要么是红色的，要么是黑色的，即：只有两种颜色
// 2. 根节点是黑色的；所有的叶子节点都是黑色的（所有的叶子节点均隐藏，即：叶子节点无作用，只是为了满足rbtree的性质）
// 3. 如果一个节点是红色的，则其两个孩子节点均为黑色的
// 4. 对任一节点，从该节点出发到其子孙节点的所有路径上的黑色节点数目均相同，即：任一节点的黑高均相同.
//    其中：定义黑高为从当前节点出发到叶子节点的路径当中黑色节点的个数。
// 上述性质保证了，从根节点到叶子节点的所有路径中，黑色节点的个数相同，且红色的节点不能相邻。

// 注意，红黑树是由2-3树，定义节点的颜色为指向该节点的边的颜色，则通过左外红连接转换过来的2-3树即为红黑树，其和2-3树是可以一一对应的

#define RBTREE_ENTRY(name, type)                                               \
    struct name {                                                              \
        unsigned char color;                                                   \
        struct type* parent;                                                   \
        struct type* left;                                                     \
        struct type* right;                                                    \
    }

#define RBTREE_NODE_CLR_RED     0
#define RBTREE_NODE_CLR_BCK     1

typedef int USER_KEY_TYPE;

typedef struct rbtree_node {
    RBTREE_ENTRY(, rbtree_node) entry;

    USER_KEY_TYPE key;
    // 下面这里就可以填写用户的value ... 
} t_rbtree_node;

typedef struct rbtree {
    t_rbtree_node *root;
    // 这里是为叶子节点服务的，将所有的叶子节点均指向这个节点，并保证这个节点是黑色的即可
    // 其是哨兵节点，与判空节点和判叶子节点是一致的操作（但是需要注意，空节点没有颜色属性） 
    t_rbtree_node *nil; 
} t_rbtree;

// 左旋操作，即：给定节点x，以节点x为中心，旋转完毕后，节点x应位于以其右孩子为根节点的左孩子位置  
// 右旋操作，即：给定节点y，以节点y为中心，旋转完毕后，节点y应位于以其左孩子为根节点的右孩子位置
/* 左右旋擦做示意图
       x                                                    y
     /   \         -> left_rotate(rbtree, x)  ->          /   \
    a     y                                              x     c
        /   \      <- right_rotate(rbtree, y) <-       /   \
       b     c                                        a     b

    左右旋转操作，共设计3个方向，6次指针的交换（因为包括parent指针和孩子指针）
    在旋转的时候注意，一定会存在的两个节点分别是x节点和y节点，其余的节点需要分情况讨论
*/
// 左右旋操作让轴心节点x或y下沉了一层，并不改变祖父节点
// 左旋操作，将当前父子节点中的较大者换到父节点上去
void rbtree_left_rotate(t_rbtree *tree, t_rbtree_node *x) {
     if (nullptr == tree || tree->nil == x)
        return;

    t_rbtree_node *y = x->entry.right;

    // 开始交换
    // 1. x的右子树，指向y的左子树
    x->entry.right = y->entry.left;
    if (tree->nil != y->entry.left) {
        y->entry.left->entry.parent = x;
    }

    // 2. y的父节点变为指向x的父节点，并交换x和y节点
    y->entry.parent = x->entry.parent;
    if (tree->nil == x->entry.parent) { // 如果x是根节点
        tree->root = y;
    } else if (x == x->entry.parent->entry.left) { // 如果x是其父节点的左孩子
        x->entry.parent->entry.left = y;
    } else { // 如果x是其父节点的右孩子
        x->entry.parent->entry.right = y;
    }

    // 3. 交换x和y的左孩子
    y->entry.left = x;
    x->entry.parent = y;
}

// 以节点y为轴心，做右旋操作
// 右旋操作是将两者中的较小者放到父节点的位置上去
void rbtree_right_rotate(t_rbtree *tree, t_rbtree_node *y) {
    if (nullptr == tree || tree->nil == y) 
        return;

    t_rbtree_node *x = y->entry.left;

    // 开始交换
    // 1. 左右子树的节点交换
    y->entry.left = x->entry.right;
    if (tree->nil != x->entry.right) {
        x->entry.right->entry.parent = y;
    }

    // 2. 祖宗节点的交换
    x->entry.parent = y->entry.parent;
    if (tree->nil == y->entry.parent) {
        tree->root = x;
    } else if (y == y->entry.parent->entry.left) {
        y->entry.parent->entry.left = x;
    } else {
        y->entry.parent->entry.right = x;
    }

    // 3. x节点和y节点本身的交换
    x->entry.right = y;
    y->entry.parent = x;
}

t_rbtree_node *rbtree_create_node(const t_rbtree *tree, USER_KEY_TYPE key) {
    if (nullptr == tree)
        return nullptr;

    t_rbtree_node *node = new t_rbtree_node;
    if (nullptr == node) 
        return nullptr;

    node->key = key;
    node->entry.left = node->entry.right = tree->nil;
    return node;
}

// 将当前节点的颜色变为红色，将其左右孩子的节点变为黑色
int rbtree_flip_color(t_rbtree_node *node) {
    if (nullptr == node)
        return -1;

    // 将两个孩子变为黑色
    node->entry.left->entry.color = RBTREE_NODE_CLR_BCK;
    node->entry.right->entry.color = RBTREE_NODE_CLR_RED;

    // 自己变为红色
    node->entry.color = RBTREE_NODE_CLR_RED;
    return 0;
}

// 插入时做红黑树平衡性调整 
// 通过左右旋转保证红黑树的重要性质：不存在两条连续的红链接和不存在红色的右链接
// 通过变色：保证黑色节点的个数相同（距离/深度相同）
void rbtree_insert_adjustment(t_rbtree *tree, t_rbtree_node *insertion) {
    if (nullptr == tree || tree->nil == insertion)
        return;
    
    t_rbtree_node *cursor = insertion;
    // 如果当前节点的父节点的颜色是红色的，其就会违反了`不存在两条连续的红链接`这一性质
    // 即此时存在4-节点，这是不行的
    // 因为插入的节点的颜色默认为是红色的
    while (cursor->entry.parent->entry.color == RBTREE_NODE_CLR_RED) {
        t_rbtree_node *grand_node = cursor->entry.parent->entry.parent;
        
        // 如果当前父节点是祖父节点的左孩子
        if (cursor->entry.parent == grand_node->entry.left) {
            t_rbtree_node *uncle_node = grand_node->entry.right;

            // 如果叔父的颜色也是红色的，则违反了`不存在红色的右链接`这一性质
            // 当两条都违反的时候，只需要将父节点、叔父节点和祖父节点的颜色变色即可恢复
            // 因为此时[parent, grand_node, uncle_node]构成了一个3-节点，且是已经分离好的状态
            // 直接变色即可
            if (uncle_node->entry.color == RBTREE_NODE_CLR_RED) {
                rbtree_flip_color(grand_node);

                // 由于grand_node变为了红色，那么可能会与上一层作用从而违反上述两种性质
                // 因此将游标设为祖父节点，并继续看看是否需要再继续调整
                cursor = grand_node;
            } else {
                // 此时叔父节点是黑色的，此时，在父节点和叔父节点这一层来看，其只违反了
                // `不存在两条连续的红链接` 这一性质；但是当看父节点和当前节点的关系时，其可能
                // 也违反了`不存在红色的右链接`这一性质，需要分情况讨论
                
                // 1. 当前的这个红色节点位于父节点的右侧，则当前节点与父节点会违反`不存在红色的右链接`这一条性质
                // 此时的情况为，cursor位于[parent, uncle_node]之间，即：插入的点是between情况，但是并不是
                // 位于同一个3-节点中，因此先合并，构造出一个3-节点再进行分裂
                if (cursor == cursor->entry.parent->entry.right) {
                    // 由于违反的是右链接，因此只需要左旋
                    // 相当于将插入的节点是右节点转变为插入的节点是左节点，即将父节点变为其左节点
                    // 因此需要提前保存一下这个“新”插入的节点。
                    cursor = cursor->entry.parent;
                    // 通过左旋操作将其调整为只违反 `不存在两条连续的红链接` 这一情况
                    // 将情况变为 cursor < parent 且 cursor < uncle_node
                    rbtree_left_rotate(tree, cursor);
                }
                
                // 2. 只违反 `不存在两条连续的红链接` 的情况
                // 即：此时cursor < parent 且 cursor < uncle_node，即：比两者都小的情况
                // 此时也构成了[cursor, parent, uncle_node]这种3-节点情况
                // 只需要将cursor->parent向上提，分裂为普通的二叉树结构即可。
                t_rbtree_node *parent = cursor->entry.parent;
                grand_node = parent->entry.parent;
                rbtree_right_rotate(tree, grand_node);
                rbtree_flip_color(parent);
            }
        } else { // 当前父节点是祖父节点的右孩子
            // uncle_node.key < grand_node.key < parent.key
            t_rbtree_node *uncle_node = grand_node->entry.right;

            if (uncle_node->entry.color == RBTREE_NODE_CLR_RED) {
                // [unclde_node, grand_node, parent]
                rbtree_flip_color(grand_node);
                cursor = grand_node;
            } else {
                //  uncle_node < grand_node < cursor < parent ?
                if (cursor == cursor->entry.parent->entry.left) {
                    cursor = cursor->entry.parent;

                    // -> [grand_node, cursor, parent]
                    rbtree_right_rotate(tree, cursor);
                }

                // uncle_node, [grand_node, cursor, parent]
                // 将grand_node作为轴心左旋，然后flip颜色即可
                t_rbtree_node *parent = cursor->entry.parent;
                grand_node = parent->entry.parent;
                rbtree_left_rotate(tree, grand_node);
                rbtree_flip_color(parent);
            }
        }
    }

    tree->root->entry.color = RBTREE_NODE_CLR_BCK;
}
 

int rbtree_insert(t_rbtree *tree, USER_KEY_TYPE key) {
    if (nullptr == tree)
        return -1;
    
    t_rbtree_node *cursor = tree->root;
    t_rbtree_node *cursor_parent = tree->nil;

    while (cursor != tree->nil) {
        cursor_parent = cursor;

        if (key < cursor->key) {
            cursor = cursor->entry.left;
        } else if (key > cursor->key) {
            cursor = cursor->entry.right;
        } else { 
            // existing node key
            return 0;
        }
    }

    t_rbtree_node *new_node = rbtree_create_node(tree, key);
    if (nullptr == new_node)
        return -2;
    
    new_node->entry.color = RBTREE_NODE_CLR_RED; 
    new_node->entry.parent = cursor_parent;
    if (cursor_parent == tree->nil) {
        tree->root = new_node;
    } else if (key < cursor_parent->key) {
        cursor_parent->entry.left = new_node;   
    } else {
        cursor_parent->entry.right = new_node;
    }
    
    // 开始调整
    rbtree_insert_adjustment(tree, new_node);
    return 0;
}

// 中序遍历
int inorder_traversal(t_rbtree *tree, t_rbtree_node *root, vector<int>& result) {
    if (nullptr == tree || nullptr == root)
        return -1;
    
    if (root != tree->nil) {
        inorder_traversal(tree, root->entry.left, result);
        result.emplace_back(root->key);
        inorder_traversal(tree, root->entry.right, result);
    }
    
    return 0;
}

int main() {
    int nums[] = {12,31,24,5,12,5,34,9,2985,324,5,69,8};
    int len = sizeof(nums) / sizeof(int);
    
    t_rbtree_node nil = {0};
    nil.entry.color = RBTREE_NODE_CLR_BCK;
    t_rbtree tree = {0};
    tree.root = &nil;
    tree.nil = &nil;

    for (int i = 0; i < len; ++i) {
        rbtree_insert(&tree, nums[i]);
    }

    vector<int> result;
    inorder_traversal(&tree, tree.root, result);
    for (auto key : result) {
        std::printf("%d ", key);
    }
    std::printf("\n");

    return 0;
}