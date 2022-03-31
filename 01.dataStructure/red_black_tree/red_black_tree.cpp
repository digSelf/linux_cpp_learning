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
// 4. 从根节点出发到达叶子节点的每条路径上的黑色节点的数目相同，即：任一路径上的黑高均相同.
//    其中：定义黑高为从当前节点出发到叶子节点的路径当中黑色节点的个数。
// 上述性质保证了，从根节点到叶子节点的所有路径中，黑色节点的个数相同，且红色的节点不能相邻。红黑树的最长路径上的节点个数与最短路径上
// 的节点个数是2倍的关系。本质上，红黑树和AVL树一样，都是通过树高来控制平衡的

// 注意，红黑树是由2-3树，定义节点的颜色为指向该节点的边的颜色，则通过左外红连接转换过来的2-3树即为红黑树，其和2-3树是可以一一对应的
// 对于调整来说，有调整策略：
// * 对于插入调整，需要站在祖父节点上来看。站在祖父节点（爷爷辈）向下看两层，看到儿子节点和孙子节点起了冲突时，才需要调整
// * 删除节点站在父节点上来看。即：站在父节点向下看一层，看两个孩子是否有冲突；
// * 插入和删除的情况处理共有5种。

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
// 插入调整主要是为了干掉双红的情况。
// 调整的原则：调整之前路径上的黑色节点数量应该等于调整之后路径上的黑色节点的数量
// 而在做旋转的时候，其并不会影响下面的子树的黑色节点的数量，即：不会影响下面子树的平衡；同理，左旋也一样。可以用归纳法证明
// 对于LL的情况，以祖父节点为轴心，向右旋转平衡；对于LR的情况，以其父节点为轴心做左旋转为LL类型后，在大右旋做颜色调整；
// 在插入节点之前，红黑树是平衡的，因此对于LL的情况，站在祖父节点往下看两层，有一些点的颜色是确定的，即：
// * 祖父节点一定是黑色的
// * 叔叔（叔父）节点是黑色的
// * 父节点是红色的，插入的孙子节点是红色的，插入节点的同辈兄弟节点一定是黑色的（因为要保证该路径要有2个黑色节点）
// * 插入的红色节点的两个孩子节点一定是黑色的（因为父节点是红色的，红色节点后如果有节点一定是黑色的，且还要保证路径上黑色节点数
// 目相同，因此这两个节点一定是存在且是黑色的）
// 上述这种确定的颜色，决定了，有两个红色节点的那一条路径上要补充一个黑色节点才行（因为叔叔节点是黑色的，在祖父和叔叔这一分支
// 上，黑色节点数量为2个），因此有两种调整方案。在大右旋完毕后，变为：红黑黑或者黑红红均可以。同理，对于RL和RR的情况与上述是一致的。
// 为了方便，统一使用红黑黑的情况来做调整（也称为红色上浮调整策略）

// 注意：在调整时，将这个部分视为整个红黑树中的一个调整到该局部的局部子树，因此局部调整的时候为了不影响全局，才会提出上述调整原则，即：在调整前，
// 每条路径上的黑色节点的数量等于调整之后的每条路径上的黑色节点的数量。在应对LR, LL, RL, RR的旋转的时候，只需要简记为：当第一个字母
// 为L的时候，一定会以祖父节点作为轴心做一个大右旋；当第一个字母为R的时候，一定会以祖父节点作为轴心做一个大左旋；而在每个情况里面，还需
// 判断一下在大右旋/大左旋前面是否要有小左旋（LR类型）/小右旋（RL类型）
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

            // 情况一：按照广度优先搜索的顺序，此时的局部情况为[黑，红，红]，此时直接变为
            // [红，黑，黑]即可，下面是具体的分析。
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
 
// 插入节点
// 插入新的节点一定是红色节点，因为如果插入黑色节点，则必然会影响某一条路径上的黑色节点数量那么一定会出现调整。
// 而插入的时红色节点的话，则可能会产生调整
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

// 搜索给定的key，返回对应的节点
t_rbtree_node *rbtree_search(t_rbtree *tree, USER_KEY_TYPE key) {
    if (nullptr == tree || tree->nil == tree->root)
        return nullptr;

    t_rbtree_node *cursor = tree->root;
    while (cursor) {
        if (key < cursor->key) {
            cursor = cursor->entry.left;
        } else if (key > cursor->key) {
            cursor = cursor->entry.right;
        } else {
            return cursor;
        }
    }

    return nullptr;
}

/*
红黑树的删除平衡调整的情况：只有在删除黑色节点的情况，才会可能触发调整。
那么问题就归结为：在删除的节点是黑色的节点，这些黑色节点都有哪些情况？通过对度（出度）的分情况讨论：
1. 删除的节点的度为0（特殊思维方式) 
2. 删除的节点的度为1
3. 删除的节点的度为2。
而删除度为2的节点可以转化为删除度为1的节点或者度为0的节点的情况，因此只需要讨论两种情况即可。需要注意调整的原则：在调整前后，路径
上的黑色节点的数目应该是相同的

# 当度为1的情况，即：这个黑色节点只有唯一一个孩子。
那么这个孩子的颜色应该是红色的。原因是：这个节点有唯一的孩子且其左右子树的黑色节点
的个数还要相等，而其因为度数为1，一定有一个孩子是空节点，那么有的那个孩子的节点的颜色一定是红色的。根据调整原则，为了保证调整前后路径上
的黑色节点数目相同，因此需要将它的孩子节点由红色变为黑色的。

# 当度为0的情况，即：这个节点没有左右孩子，只有`nil`节点。在删除的时候，这个节点是指向了`nil`节点，根据调整原则，路径上删除前这个局部
的黑色节点个数是2，那么删除后将这个`nil`节点标记为了双重黑色，虽然只有1个`nil`节点，但是它却记2次了。这种情况就需要进行删除调整，即：
删除调整就是用来干掉这种双重黑色的情况。

总结：
* 删除红色的节点，不影响红黑树的平衡
* 度为1的节点，其孩子一定是红色的，删除后将红色节点改为黑色即可，也不会影响红黑树的平衡
* 删除度为0的节点，会产生一个双重黑的nil节点。删除调整因此产生，删除调整就是为了干掉这个双重黑的

通过不同的删除情况，来做调整：

删除的情况1：站在双重黑的节点x的父节点y向下看，这个x节点的兄弟节点z也是黑色的，且节点z的两个孩子也是黑色的，即：双重黑的兄弟是黑色的，且
其两个孩子也是黑色的。处理方法为：将父节点x加一重黑色；将其兄弟节点z减去一重黑色（即：变为红色）；最后自己身上的双重黑变为一重黑（减一重黑）

删除的情况2：双重黑节点的兄弟节点是黑色的，且兄弟节点的子节点有红色的节点。此时对兄弟节点按照AVL的不平衡的情况（LR，RL，LL，RR）进行分情况讨论：
* 兄弟节点在右侧，红色孩子节点在兄弟节点的右侧，即RR的情况。先一个大左旋，然后将新得到的子树的根节点变为原来根节点（祖父节点）的颜色，其两个子节点变
为黑色，然后将双黑节点减一重黑色
* 兄弟节点在左侧，红色孩子节点在兄弟节点的左侧，即LL的情况。与RR的情况是一直的，只不过操作反过来。注意：RR类型和LL类型不管另一个孩子节点的颜色，只要
满足上面上面定义，他就是RR和LL的

* 兄弟节点在右侧，红色孩子节点在兄弟节点的左侧，且其右侧孩子节点的颜色必须是黑色的，此时称为RL情况。通过一次小右旋，肯定能变为RR的情况。在小右旋后，
将根节点变为黑色，其右孩子变为红色后，就变为了RR的情况了。
* 兄弟节点在左侧，红色节点在兄弟节点的右侧，且其左侧孩子节点的颜色必须是黑色的，此时为LR情况，通过一次小左旋，将根节点变为黑色，其做好自己变为红色后
，转为LL的情况了。

删除的情况3：双黑节点的兄弟节点是红色的。将红色的兄弟节点旋转到根节点上来。然后将根节点变为红色，如果向左选择，则左孩子变黑色；否则右孩子变黑色。此时
情况川味上述的两种情况了，即：兄弟节点是黑色的情况了。

总结一下：
1. 双重黑的兄弟是黑色的，且其两个孩子也是黑色的。双重黑和兄弟节点的父节点加一重黑，双黑节点和兄弟节点都减一层黑
2. 当双黑节点的兄弟是黑色的，且兄弟节点有孩子节点是红色的
    * R（兄弟节点在父节点的右子树上）R（红色右子节点），大左旋，新根变为原根颜色，将新根的两个子节点改为黑色
    * RL。先小右旋，对调原根和新根的颜色，变为RR的情况
    * LL同RR，对称操作
    * LR同RL，对称操作
3. 如果兄弟节点是红色的，先小右/左旋转，然后将新根节点变为黑色，原根节点变为红色，转为双黑节点的兄弟节点是黑色的情况。
*/


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

    std::printf("%d %d\n", rbtree_search(&tree, 13) != nullptr, 
                           rbtree_search(&tree, 5) != nullptr);

    return 0;
}