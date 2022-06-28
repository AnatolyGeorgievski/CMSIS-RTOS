#include "r3_tree.h"

/*! \brief находит элемент дерева и перемещает элемент ближе к корню дерева

Эта операция выполняется как для обычного бинарного дерева, только после нее запускается операция splay.
*/
tree_t* tree_find(tree_t** root, uint32_t key)
{
    tree_t *grande = NULL;
    tree_t *parent = NULL;
    tree_t *node   = *root;
    while(node) {
		int32_t cmp = key - node->key;
		if(cmp==0) break;//return node->value;
		grande = parent;
		parent = node;
		/* если найден дикий дисбаланс, то можно балансировать */
        node= (cmp < 0)? node->prev: node->next;
    }
    if (node==NULL) return node;// не найден, NULL
    if (parent!=NULL /* && баланс улучшается? */) {
        node = parent;
        if (parent->prev == node) {
            node = tree_rotate_right(parent);
        } else {// if (parent->next == node)
            node = tree_rotate_left (parent);
        }
        if (grande==NULL)
            *root = node;
        else if (grande->next == parent) {
            grande->next = node;
            grande->next_depth = _depth(node);
        } else {
            grande->prev = node;
            grande->prev_depth = _depth(node);
        }
    }
    return node;
}
