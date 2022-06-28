#include "r3_tree.h"

/*! \brief альтернативная реализация алгоритма удаления из дерева

    можно опустить удаляемый элемент пока у него не останестся
    один или ни одного последователя, тогда его можно исключить из дерева.
*/
tree_t* tree_remove(tree_t **root, uint32_t key)
{
    tree_t *node = *root;
    tree_t *parent = NULL;
    while(node) {
		register int32_t cmp = key - node->key;
		if(cmp==0) break;
		parent = node;
        node = (cmp < 0)? node->prev: node->next;
    }
    if (node!=NULL) {// нашли
        if (parent==NULL) {// элемент находится на вершине
            *root = tree_merge(node->prev, node->next);
        } else
        while (1) {
            if (parent->next == node) {
                if (node->next==NULL) {
                    parent->next = node->prev;
                    parent->next_depth = node->prev_depth;
                    break;
                }
                if (node->prev==NULL) {
                    parent->next = node->next;
                    parent->next_depth = node->next_depth;
                    break;
                }
            //выполнить вращение и спуск по дереву
                if (_balance(node) >= 0) {// право лучше
                    parent->next = tree_rotate_left (node);
                } else {
                    parent->next = tree_rotate_right(node);
                }
                parent->next_depth = _depth(parent->next);
                parent = parent->next;
            } else {// if (parent->prev == node)
                if (node->next==NULL) {
                    parent->prev = node->prev;
                    parent->prev_depth = node->prev_depth;
                    break;
                }
                if (node->prev==NULL) {
                    parent->prev = node->next;
                    parent->prev_depth = node->next_depth;
                    break;
                }
                if (_balance(node) >= 0) {// право лучше
                    parent->prev = tree_rotate_left (node);
                } else {
                    parent->prev = tree_rotate_right(node);
                }
                parent->prev_depth = _depth(parent->prev);
                parent = parent->prev;
            }
        }

        node->prev =NULL;
        node->next =NULL;
        node->prev_depth =0;
        node->next_depth =0;
    }
    return node;
}
