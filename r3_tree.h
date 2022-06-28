/*! */
#ifndef R3_TREE_H
#define R3_TREE_H
//#include <glib.h>
#include <stdint.h>
#include "r3_slice.h"
typedef struct _ttl_datalist ttl_datalist_t;
typedef struct _ttl_list ttl_list_t;
struct _ttl_datalist {
    //uint32_t timestamp;
    ttl_list_t* list;
//    volatile int32_t version; // ревизия базы данных устройства
//    volatile int32_t ref_count;
};
void* ttl_datalist_get  (ttl_datalist_t * dl, uint32_t key);
void* ttl_datalist_set  (ttl_datalist_t * dl, uint32_t key, void* value);
void  ttl_datalist_push (ttl_datalist_t * dl, uint32_t key, void* value);
void* ttl_datalist_replace (ttl_datalist_t * dl, uint32_t key, void* value);
void* ttl_datalist_pop  (ttl_datalist_t * dl, uint32_t key);
void  ttl_datalist_clear(ttl_datalist_t * dl);
void* ttl_datalist_get_next(ttl_datalist_t * dl, uint32_t *key, void** iter);
void* ttl_datalist_get_iter(ttl_datalist_t * dl, uint32_t key, void** iter);
//void* ttl_datalist_foreach(ttl_datalist_t * dl, int (*test_cb)(void* , void* ), void* user_data);





typedef struct _tree tree_t;
struct _tree {
    tree_t* prev;	//!< левое поддерево
    tree_t* next;	//!< правое дерево
    void * value;
    uint32_t  key;	//!< ключ
    uint16_t  prev_depth;//!< число элементов дерева слева
    uint16_t  next_depth;//!< число элементов дерева справа

};

typedef void (*tree_serialize_cb)(/*tree_t * leaf, */  uint32_t key, void* value, void* user_data);

tree_t* tree_init(tree_t *leaf, uint32_t key, void* value);
tree_t* tree_insert(tree_t** root, tree_t* elem);
tree_t* tree_insert_tree (tree_t** root, tree_t* elem);
tree_t* tree_find  (tree_t** root, uint32_t key);
void* tree_lookup(tree_t *node, uint32_t key);
#if 0
static inline void* tree_lookup(tree_t *node, uint32_t key)
{
    while(node) {
		int32_t cmp = key - node->key;
		if(cmp==0) return node->value;
        node = (cmp < 0)? node->prev: node->next;
    }
    return NULL;
}
#endif
void* tree_replace_data(tree_t *node, uint32_t key, void* value);

uint16_t  tree_balance(tree_t **root);
/*! \brief найти первый попавшийся удовлетворяющий маске

    \note Чтобы найти все элементы одного типа, надо сначала выделить узел корень по маске,
    потом выполнить обход всех элементов в под-дереве, таким образом можно сортировать и группировать адреса с учетом масок.
 */
static inline tree_t* tree_lookup_mask(tree_t *node, uint32_t key, int shift)
{
    while(node) {
		int32_t cmp = (int32_t)(key - node->key) >> shift;
		if(cmp==0) break;
        node = (cmp < 0)? node->prev: node->next;
    }
    return node;
}

/*! \fn tree_t* tree_max(tree_t* node)
    \brief возвращает самый маленький элемент в дереве относительно заданого узла
*/
static inline tree_t* tree_min(tree_t *node) {
    while(node->prev) node = node->prev;
    return node;
}
/*! \fn tree_t* tree_max(tree_t* node)
    \brief возвращает самый большой элемент в дереве
*/
static inline tree_t* tree_max(tree_t *node) {
    while(node->next) node = node->next;
    return node;
}

tree_t* tree_remove (tree_t **root, uint32_t key);
void    tree_notify (tree_t *root, void (*notify)(tree_t * leaf,  void* user_data), void* user_data);
void    tree_foreach(tree_t*root, tree_serialize_cb , void* user_data);
tree_t* tree_merge(tree_t *tree1, tree_t *tree2);

/*! \brief функция фактически возвращает число элементов
    в дереве
*/
static inline uint16_t _depth(tree_t* node)
{
	return node->next_depth + node->prev_depth +1;
//	return node->prev_depth >= node->next_depth? node->prev_depth+1: node->next_depth+1;
}
static inline int16_t _balance(tree_t *node) {
	return node->next_depth - node->prev_depth;
}

/*! \brief вращение дерева вправо
    [n]        [p]
	/ \        / \
  [p] [c] => [a] [n]
  / \            / \
[a] [b]        [b] [c]
*/
static inline tree_t* tree_rotate_right(tree_t* node)
{
	tree_t * pivot = node->prev;
	node->prev = pivot->next;
	pivot->next = node;

	node->prev_depth = pivot->next_depth;
	pivot->next_depth = _depth(node);
	return pivot;
}
/*! \brief вращение дерева влево
    [n]          [p]
	/ \          / \
  [a] [p]  =>  [n] [c]
	  / \      / \
	[b] [c]  [a] [b]
*/
static inline tree_t* tree_rotate_left(tree_t* node)
{
	tree_t* pivot = node->next;
	node->next = pivot->prev;
	pivot->prev = node;

	node->next_depth = pivot->prev_depth;
	pivot->prev_depth = _depth(node);
	return pivot;
}

#endif // R3_TREE_H

