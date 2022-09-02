#include "r3_tree.h"

/*! \brief инициализация элемента дерева

В нашей конецпции нет этапа выделения памяти под элементы дерева.
После выделения памяти, следует использовать функцию инициализации.
 */
tree_t* tree_init(tree_t *leaf, uint32_t key, void* value)
{
	leaf->key  = key;
	leaf->value= value;
	leaf->prev = NULL;
	leaf->next = NULL;
#if 0
	leaf->prev_depth= 0;
	leaf->next_depth= 0;
#endif
	return leaf;
}
