#include "r3_tree.h"

/*! \brief функция вставки с балансировкой на последнем элементе */
tree_t* tree_insert(tree_t** root, tree_t* elem)
{
    tree_t* node = *root;
    tree_t* grande = NULL;
    tree_t* parent = NULL;
    while(node) {
        int32_t cmp = elem->key - node->key;
		if (cmp== 0){ // добавляем без изменения структуры дерева
			void* value = node->value;
			node->value = elem->value;
			elem->value = value; 
//			elem->value = atomic_exchange(&node->value, elem->value);
			return elem;// элемент надо удалить или использовать повторно
		}
        if (cmp < 0) {
            if (node->prev==NULL) {// добавить слева
                if (node->next!=NULL || parent==NULL) {// не удлиняем дерево
                    node->prev = elem;
                } else if (parent->prev==NULL) {    // вставка слава от node
                    parent->next = NULL;            //   [p]        [e]
                    elem->prev = parent;            //   / \   =>   / \   .
                    elem->next = node;              //     [n]    [p] [n]
                    if (grande==NULL)
                        *root = elem;
                    else if (grande->prev==parent)
                        grande->prev = elem;
                    else //if (grande->next==parent)
                        grande->next = elem;
                    return NULL;
                } else if (parent->next==NULL) {    // вставка слава от node
                    parent->prev = NULL;            //   [p]        [n]
                    node->next = parent;            //   / \   =>   / \   .
                    node->prev = elem;              // [n]        [e] [p]
                    if (grande==NULL)
                        *root = node;
                    else if (grande->prev==parent)
                        grande->prev = node;
                    else //if (grande->next==parent)
                        grande->next = node;
                    return NULL;
                } else {
                    node->prev = elem;
                }
				break;
            } else {
                grande = parent;
                parent = node;
                node = node->prev;
            }
        }
		else {// if (cmp>0)
            if (node->next==NULL) {
                if (node->prev!=NULL || parent==NULL) {// удлиняем дерево
                    node->next = elem;
                } else if (parent->prev==NULL) {    // вставка справа от node
                    parent->next = NULL;            //  [p]        [n]
                    node->prev = parent;            //  / \   =>   / \   .
                    node->next = elem;              //    [n]    [p] [e]
                    if (grande==NULL)
                        *root = node;
                    else if (grande->prev==parent)
                        grande->prev = node;
                    else//if (grande->next==parent)
                        grande->next = node;
                    return NULL;
                } else if (parent->next==NULL) {    // вставка справа от node
                    parent->prev = NULL;            //   [p]       [e]
                    elem->next = parent;            //   / \  =>   / \  .
                    elem->prev = node;              // [n]       [n] [p]
                    if (grande==NULL)
                        *root = elem;
                    else if (grande->prev==parent)
                        grande->prev = elem;
                    else//if (grande->next==parent)
                        grande->next = elem;
                    return NULL;
                } else {// этот вариант не реализуется, не должен
                    node->next = elem;
                }
				break;
            } else {
                grande = parent;
                parent = node;
                node = node->next;
            }
        }
//		depth++;
    }
	return NULL;
}
