#include "r3_tree.h"
/*!
    \brief функция может использоваться для сериализации данных

*/
//__attribute__((optimize("O2")))
void  tree_notify(tree_t *root, void (*notify)(tree_t * leaf,  void* user_data), void* user_data)
{
	tree_t * prev = root->prev;
	tree_t * next = root->next;
	notify (root,user_data);
	if (prev) tree_notify(prev, notify, user_data);
	if (next) tree_notify(next, notify, user_data);
}
