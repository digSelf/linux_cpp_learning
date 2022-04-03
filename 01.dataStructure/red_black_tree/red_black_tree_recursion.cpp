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

// 返回指定节点的前驱节点
t_rbtree_node *__rbtree_find_predecessor(t_rbtree_node *node) {
    t_rbtree_node *cursor = node->left;
    while (cursor->right != nil_node) {
        cursor = cursor->right;
    }
    
    return cursor;
}

/*
    调整前提：待平衡子树中有双黑节点且当前节点是双黑节点的父节点。设当前待平衡子树的根节点为双黑节点的父节点，记为root。
双黑节点记为x，其兄弟节点记为y。分析调整策略及颜色变化规则时应该通过分类讨论和归纳证明的思想一起来做。平衡的目标是消除双
黑节点，并保证从整颗树的根节点到叶子节点的所有路径上的黑色节点数量相等。
    由于当前子树是以root为根节点的，此时root的颜色有两种可能，分别是黑色或者红色；双黑节点的兄弟节点可能是红色也可能是黑
色的；兄弟节点的子孩子可能都是黑色的，也可能是一黑一红，但不可能是全是红色。因为当前在删除当前节点之前，当前整棵树已经是红
黑树了，根据上面的插入和插入调整的方式，当一个节点的两个子孩子全是红色的时候，会强制变成红黑黑的情况。因此可能出现的分支结构
共有2 * 2 * 2 = 8种可能。而这8种可能中，也有一些不可能出现的情况，因此经过具体分析只会产生3种大类。
    
    # 情况1.当双黑节点的兄弟节点是红色的情况。此时root的颜色一定是黑色的，因为删除之前整棵树是红黑树，其各个路径上的黑节点的
数目应该相同。而在删除的时候，如果删除的节点是红色，此时并不会影响路径上的黑色节点数目；如果删除的是黑色，此时其在删除的路径
上产生一个双重黑的节点，也不会改变该路径上的黑色节点数目。而如果root节点是红色的，且双黑节点x的兄弟节点y也是红色的话，会与
此时的各路径上的黑色节点数目相同这一条件相冲突，通过反证法可知，此时root节点的颜色一定是黑色的。
    对于情况1可以通过小左/右旋将红节点y上浮，将双黑节点的兄弟变为黑色的。此时由于红色节点上浮变为新的根节点，原根节点下沉导致
公共路径上少了一个黑色节点，而双黑节点一侧又多了一个黑色的节点，因此为了保证调整前后各路径上的黑节点数目相同，需要：将新根节点
变为黑色，原根节点变为红色。
    经过上述调整，此时的情况可能会变为下述情况2或情况3.

    # 情况2. 当双黑节点的兄弟节点是黑色节点，且兄弟节点的子孩子有一个是红色的，而另一个子孩子必须是黑色的且与兄弟节点y的方向相反，
    此时根节点root的颜色可以是红色，也可以是黑色的。此时又分为多种情况：RL类型或LR类型。通过小左旋/右旋可以变为情况3.
    当旋转完后，由于红色节点上浮，导致路径上少了一个公共的黑色根节点y，因此新根节点应该变为黑色；而另一侧由于原黑色根节点下沉多出一个
要将原根节点的颜色变为红色，此时就变为RR/LL的情况了，即：旋转后，原根节点变红，新根节点变黑即可。
    当是RL类型，即兄弟节点是root节点的右孩子，红色子节点是兄弟节点y的左孩子的情况（注意此时兄弟节点y的右孩子必须是黑色的，原因
是如果此时兄弟节点y的右孩子是红色的话，应隶属于情况3 - RR的情况）。此时可以通过小右旋，变为RR的情况，也就是情况3
    当时LR类型，即兄弟节点是root节点的左孩子，红色子节点是兄弟节点y的右孩子的情况，（注意此时兄弟节点y的左孩子必须是黑色的，原因
是如果此时兄弟节点y的左孩子是红色的话，应隶属于情况3 - LL的情况），此时可以通过小左旋，变为LL的情况，也就是情况3

    # 情况3. 当双黑节点的兄弟节点是黑色节点，且兄弟节点的子孩子有一个是红色的，而另一个子孩子的颜色无所谓且和兄弟节点的方向同向，
即：兄弟节点y是root节点的右孩子/左孩子，则它的红色子孩子的方向也是右孩子/左孩子。
    由于要保证在旋转后黑色节点的数目要与旋转前相同，而root的颜色是不确定的，当通过大左旋/右旋，其均会将原根节点root下沉，双黑节点x
的兄弟节点y上浮成新的公共黑色根节点。如果root是红色，此时双黑节点路径上一侧会增加1个黑色节点数目；如果root是黑色，此时双黑节点路径
上一侧会增加两个黑色节点数目。为了平衡，需要将新根节点变为原根节点的颜色，新根节点的两个孩子均改为黑色，双重黑节点减去一重黑即可。
*/

#define RBTREE_ERASE_CLT_NONE  0
#define RBTREE_ERASE_CLT_LEFT  1
#define RBTREE_ERASE_CLT_RIGHT 2
// 给定一颗树的根节点，将该树重新平衡成一棵红黑树，并返回平衡后的树的根节点
t_rbtree_node *__rbtree_erase_maintain(t_rbtree_node *root) {
    // 调整时，站在父节点来进行调整；而维护的时候主要是站在父节点向下看，看有没有双重黑的子节点
    if (root->left->color != RBTREE_CLR_DBL && root->right->color != RBTREE_CLR_DBL) {
        return root;
    }
    
    // 如果有双重黑节点，则分为两大类来进行分情况讨论：1. 兄弟节点是黑节点 2. 兄弟节点是红节点
    // 对于第2种情况，可以通过小旋转，转变为兄弟节点是黑节点这种情况；注意在旋转完毕后仍然保证其红黑树的性质，因此需要变颜色
    // 修改方法是：新根节点变黑色，原根节点变红色。根据调整原则，调整前的黑色节点数为3，调整后的黑色节点数也应该相同。而由于旋转
    // 其原黑色的跟节点下沉了，导致有一侧路径上缺少一个公共的黑色节点，因此新根节点必须变为黑色；而另一侧由于下沉的黑色节点导致多了一个
    // 因此，下沉的原根节点为了不影响调整原则，它必须变成红色。
    if (has_red_child_node(root)) { // 由于上面已经判断了是否有双重黑节点了，如果root还有红色子节点，证明当前的情况是情况2
        int has_red_conflict = RBTREE_ERASE_CLT_NONE;
        root->color = RBTREE_CLR_RED;
        if (root->left->color == RBTREE_CLR_RED) {
            // 如果左孩子是红色的，那么这个红色孩子的两个黑孩子，需要通过小右旋才能使得双黑节点的兄弟节点是黑色的，变为情况1
            root = rbtree_right_rotate(root);
            has_red_conflict = RBTREE_ERASE_CLT_LEFT;
        } else {
            // 同上
            root = rbtree_left_rotate(root);
            has_red_conflict = RBTREE_ERASE_CLT_RIGHT;
        }
        root->color = RBTREE_CLR_BLK;

        // 判断双黑节点在哪头，然后递归进行处理
        if (has_red_conflict == RBTREE_ERASE_CLT_LEFT) {
            // 双黑节点在右侧，因此需要向右递归处理
            root->right = __rbtree_erase_maintain(root->right);
        } else {
            root->left = __rbtree_erase_maintain(root->left);
        }

        return root;
    }

    // 此时变为情况1了，双黑节点的兄弟节点是黑色的。即：当前节点root下没有红色节点，等价于当前节点下的两个孩子节点都是黑色且其中有一个是双重黑
    // 1.1 兄弟节点没有红色子孩子，此时的具体情况为：根节点是黑色节点，双重黑节点的兄弟节点也是黑色的，且兄弟节点的两个孩子也是黑色的
    // 调整方法为：兄弟节点和双重黑节点均减一重黑，根节点加一重黑。因为根据调整原则，双重黑肯定是不能存在的，一定要减掉一重，那么根节点
    // 加一重才能保证这一侧的节点数调整前后相同；而由于公共的根节点的黑色加了一重，那么双重黑节点的兄弟节点颜色一定要减去一重，否则就多一重了
    if ((root->left->color == RBTREE_CLR_DBL && !has_red_child_node(root->right)) || 
        (root->right->color == RBTREE_CLR_DBL && !has_red_child_node(root->left))) {
        
        root->left->color -= RBTREE_CLR_BLK;
        root->right->color -= RBTREE_CLR_BLK;

        root->color += RBTREE_CLR_BLK;
        
        // 注意删除调整时站在父节点上看，此时虽然当前的root是双重黑节点了，但是它自己处理不了，需要让他的父节点进行处理
        // 因此直接返回当前的根节点位置即可，如果此时已经是整棵树的全局根节点了，也没事儿，最外层会强制把根节点变为一重黑
        // 相当于是进行了调整，也满足情况了
        return root;
    }

    // 此时，判断情况3
    if (root->left->color == RBTREE_CLR_DBL) { // 双重黑节点是在根节点的左侧，兄弟节点在右侧
        // 判断是否需要经过小右旋
        if (root->right->left->color == RBTREE_CLR_RED) {
            // 原根节点变红
            root->right->color = RBTREE_CLR_RED;
            root->right = rbtree_right_rotate(root->right);
            // 新根节点变黑
            root->right->color = RBTREE_CLR_BLK;
        }

        root->left->color -= RBTREE_CLR_BLK;
        // 最后一定要经过一个大左旋变平衡
        root = rbtree_left_rotate(root);
        // 新根节点变为原根节点的颜色
        root->color = root->left->color;
    } else { // 双重黑节点是在根节点的右侧，兄弟节点在左侧
        if (root->left->right->color == RBTREE_CLR_RED) {
            root->left->color = RBTREE_CLR_RED;
            root->left = rbtree_left_rotate(root->left);
            root->left->color = RBTREE_CLR_BLK;
        }

        // 双重黑节点减去一重黑
        root->right->color -= RBTREE_CLR_BLK;
        root = rbtree_right_rotate(root);
        root->color = root->right->color;
    }
    
    // 两个子节点要变为黑色
    root->left->color = root->right->color = RBTREE_CLR_BLK;

    return root;
}

// 给定树的根节点，删除指定key的节点，并返回指向当前根节点的指针
t_rbtree_node *__rbtree_erase(t_rbtree_node *root, USER_KEY_TYPE key) {
    if (root == nil_node) // 如果为空节点，则当前没有想要删除的节点的值
        return nil_node;
    
    if (key < root->key) {
        root->left = __rbtree_erase(root->left, key);
    } else if (key > root->key) {
        root->right = __rbtree_erase(root->right, key);
    } else {
        // 存在要删除的节点，且当前节点即为要删除的节点 
        // 先处理度为0或者度为1的节点
        if (root->left == nil_node || root->right == nil_node) {
            // 如果进入了该条件体中，则证明其满足了3中情况
            // 如果是度为1的节点，则返回的是其不为nil的孩子的指针；否则返回的是指向nil_node的指针
            t_rbtree_node *child = (root->left != nil_node ? root->left : root->right);

            // 处理方法是将当前要删除的节点（root节点）的颜色加到孩子节点上去
            // 如果当前要删除的节点是红色，其宏值为0，不改变孩子节点的颜色；如果删除的是黑色节点，则孩子节点加一重黑
            // 如果此时child是nil_node，则nil_node变为双重黑
            child->color += root->color; 

            // 删除前树的形状为  root              root 
            //                /         或          \
            //             child                   child
            //             /   \                    /   \
            //            n1   n2                  n1   n2
            SAFE_DELETE_NODE(root);

            // 删除当前节点root后，child变为了树的根节点了
            return child;
        }

        // 此时是度为2的情况，跟AVL树删除度为2的节点的方法是一样的，先找到这个节点的前驱节点来覆盖当前要删除的节点
        // 然后逻辑转为删除这个前驱节点，从而转变为上述度为0或1的情况
        t_rbtree_node *prev_node = __rbtree_find_predecessor(root);
        root->key = prev_node->key;
        
        // 由于前驱节点在当前节点的左子树中，所以流程转为在当前节点的左子树中删除值为key的节点
        root->left = __rbtree_erase(root->left, prev_node->key);
    }

    // 维护红黑树是在回溯期间发生的
    return __rbtree_erase_maintain(root);
}

// 删除节点
void rbtree_erase(t_rbtree *tree, USER_KEY_TYPE key) {
    if (tree == nullptr) 
        return;

    tree->root = __rbtree_erase(tree->root, key);
    tree->root->color = RBTREE_CLR_BLK;
}

// 中序遍历的内部封装函数
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

    // 删除操作
    int del_nums[] = {5, 0, 12, 2985, 69};
    len = sizeof(del_nums) / sizeof(int);
    for (int i = 0; i < len; ++i) {
        rbtree_erase(&tree, del_nums[i]);
    }
    
    result.clear();
    inorder_traversal(&tree, result);
    for (auto key : result) {
        std::printf("%d ", key);
    }
    std::printf("\n");

    return 0;
}