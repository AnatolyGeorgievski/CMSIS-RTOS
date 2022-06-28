#include "r3_tree.h"
/*! \brief обход дерева слева направо сериализация данных по возрастанию */
//__attribute__((optimize("O2")))
void tree_foreach(tree_t* root, void (*serialize)(/*tree_t * leaf, */  uint32_t key, void* value, void* user_data), void* user_data)
{
	if (root->prev) tree_foreach(root->prev, serialize, user_data);
	serialize (root->key, root->value, user_data);
	if (root->next) tree_foreach(root->next, serialize, user_data);
}
