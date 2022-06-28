#include "r3_tree.h"
/*! \brief поиск по дереву
    \return значение
*/
void* tree_lookup(tree_t *node, uint32_t key)
{
    while(node) {
		int32_t cmp = key - node->key;
		if(cmp==0) return node->value;
        node = (cmp < 0)? node->prev: node->next;
    }
    return NULL;
}
/*
tree_t* tree_lookup1(tree_t **prev, uint32_t key)
{
	tree_t *node;
    while((node=*prev)!=NULL) {
		int32_t cmp = key - node->key;
		if(cmp==0) return node;
		else 
		if(cmp < 0)
			prev = &node->prev;
		else prev = &node->next;
    }
    return NULL;
}
*/