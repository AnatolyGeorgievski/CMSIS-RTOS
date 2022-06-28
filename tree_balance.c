#include "r3_tree.h"
#pragma GCC optimize("Os")
/*! полная балансировка дерева, может выполняться пока читателей нет
    функция баланса может работать как расчет приоритета
 */
static uint16_t  tree_balance_(tree_t **root_ref) {
	tree_t *node = *root_ref;
	int balance = _balance(node);
	if (balance < -1) {
		if (/*node->prev!=NULL && */_balance(node->prev) > 0) {
			node->prev = tree_rotate_left (node->prev);
		}
		node = tree_rotate_right (node);
		*root_ref = node;
    }
	else if (balance > 1) {
		if (/*node->next!=NULL &&  */_balance(node->next) < 0) {
			node->next = tree_rotate_right (node->next);
		}
		node = tree_rotate_left (node);
		*root_ref = node;
    }
	return _depth(node);
}

//__attribute__((optimize("O2")))
/*! \brief выполнить балансировку дерева
    Балансировка узла выполняется по параметру число элементов слева и справа.

    \return число элементов в дереве. Условно.
 */
uint16_t  tree_balance(tree_t **root_ref)
{
	tree_t *node = *root_ref;
#if 1
	if (node->prev==NULL) {								//
        if (node->next==NULL) return 1;					//  
        *root_ref = node = tree_rotate_left (node);		//  [n]   =>  [r]
	}													//  / [r]   [n]
	node->prev_depth = tree_balance(&node->prev);
	if (node->next == NULL){
        if (node->prev==NULL) return 1;
        *root_ref = node = tree_rotate_right (node);
	}
	node->next_depth = tree_balance(&node->next);
#else
	node->prev_depth = (node->prev==NULL)? 0: tree_balance(&node->prev);
	node->next_depth = (node->next==NULL)? 0: tree_balance(&node->next);
#endif
	return tree_balance_(root_ref);
}
/*! \brief Пересчет глубины (весов)
 */
uint16_t  tree_depth(tree_t *node) 
{
	node->prev_depth = node->prev==NULL? 0: tree_depth(node->prev);
	node->next_depth = node->next==NULL? 0: tree_depth(node->next);
	return _depth(node);
}
