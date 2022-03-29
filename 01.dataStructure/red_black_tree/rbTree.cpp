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

// 红黑树的4条性质：
// 1. 所有的节点要么是红色的，要么是黑色的，即：只有两种颜色
// 2. 根节点是黑色的；所有的叶子节点都是黑色的（所有的叶子节点均隐藏，即：叶子节点无作用，只是为了满足rbtree的性质）
// 3. 如果一个节点是红色的，则其两个孩子节点均为黑色的
// 4. 对任一节点，从该节点出发到其子孙节点的所有路径上的黑色节点数目均相同，即：任一节点的黑高均相同.
// 上述性质保证了，从根节点到叶子节点的所有路径中，黑色节点的个数相同，且红色的节点不能相邻。

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

// 插入时做红黑树平衡性调整 
// 在插入某一个节点以前，这棵树是红黑树，即满足红黑树的性质。那么在插入这个节点后，这个节点是什么颜色好？
// 红色好，因为黑色会改变黑高。即：插入的节点是红色的话更容易满足红黑树的性质（改变黑高后，可能需要调整树）
// 那在什么情况下是需要调整这颗红黑树的呢？只有违背`3. 如果一个节点是红色的，则其两个孩子节点均为黑色的`
// 这一条性质的时候，才需要调整红黑树。即：当插入的这个节点是红色的，且其父节点也是红色的情况下需要调整。
// 那么红黑树都有哪些形状是插入的节点是红色的，其父节点也是红色的？设当前插入的节点为红色的，则有：
// 1. 当前节点是红色的，其父节点是红色的，其叔父节点（父节点的兄弟节点）也是红色的。(当前节点可能是父节点的左孩子或右孩子2种情况)
// 2. 当前节点是红色的，其父节点是红色的，其叔父节点是黑色的。这种情况不太正常，基因不太对（比如白人祖父，黑人叔父）
//    此时只能通过旋转解决，即以插入的这个节点所在这一层画一条水平线，这一层的兄弟节点到其到祖父的黑高是不相同的，叔父那头重了。
//    因此只能通过先左旋后右旋搞定
void rbtree_insert_adjustment(t_rbtree *tree, t_rbtree_node *insertion) {
    if (nullptr == tree || tree->nil == insertion)
        return;
    
    // 在做调整时，只有违背第四条，即：其父节点也是红色时需要调整
    t_rbtree_node *cursor = insertion;
    while (cursor->entry.parent->entry.color == RBTREE_NODE_CLR_RED) {
        // 分情况讨论：
        // 1. 其父节点是祖父节点的左孩子
        t_rbtree_node *grand_node = cursor->entry.parent->entry.parent;
        if (cursor->entry.parent == grand_node->entry.left) {
            t_rbtree_node *uncle_node = grand_node->entry.right;

            if (uncle_node->entry.color == RBTREE_NODE_CLR_RED) {
                // 此时的情况是：插入的节点是红色，父节点是红色，叔父节点是红色
                cursor->entry.parent->entry.color = RBTREE_NODE_CLR_BCK;
                uncle_node->entry.color = RBTREE_NODE_CLR_BCK;

                grand_node->entry.color = RBTREE_NODE_CLR_RED;
                cursor = grand_node;
            } else {
                // 此时的情况是：插入的节点是红色，父节点是红色，叔父节点是黑色
                // 分为两种情况：插入的节点是父节点的左孩子和插入的节点是父节点的右孩子
                if (cursor == cursor->entry.parent->entry.right) {
                    cursor = cursor->entry.parent;
                    rbtree_left_rotate(tree, cursor);
                }

                cursor->entry.parent->entry.color = RBTREE_NODE_CLR_BCK;
                grand_node->entry.color = RBTREE_NODE_CLR_RED;
                rbtree_right_rotate(tree, grand_node);
            }

        } else {
            // 2. 插入节点的父节点是祖父节点的右孩子
        }
    }
}
 

int rbtree_insert(t_rbtree *tree, USER_KEY_TYPE key) {
    if (nullptr == tree)
        return -1;
    
    t_rbtree_node *cursor = tree->root;
    t_rbtree_node *cursor_parent = tree->nil;

    while (cursor) {
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