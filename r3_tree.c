#include <stdint.h>
//#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include "atomic.h"
#include "r3_tree.h"
// https://neerc.ifmo.ru/wiki/index.php?title=Splay-%D0%B4%D0%B5%D1%80%D0%B5%D0%B2%D0%BE
//#include "r3_slice.h"
/*! про это дерево известно что идентификаторы назначаются подряд,
т.е. при добавлении объекта можно сделать одно допущение,
балансировать дерево на обратном пути при добавлении объекта
балансировка делается при помощи двух функций вращения: влево и вправо.

дерево создается из архива и сохраняется в архив периодически.
в архиве дерево хранится таким образом, чтобы процесс восстановления из архива не требовал балансировки дерева.
В нашей ОС деревья используются для доступа к конфигурации устройства.

Особенность реализации, дерево может быть статчиеским, механизм выделения памяти не регламентируется.

Нужны параметры для балансировки и использования дерева
1) Число элементов в дереве
2) Число свободных индексов, мы не хотим удалять индексы и узлы в дереве
3) Глубина дерева,
 */

/*! \brief редуцирование дерева правая сторона

операция выполняется при добавлении последнего элемента в список при наличии parent
if (p->next==NULL && g->prev==NULL) {
	g->next = NULL;
	elem->prev = g;
	elem->next = p;
	return elem;
}
    [g]          [l]
	/ \          / \
      [p]  =>  [g] [p]
	  / \
	[l]

if (p->next==NULL && g->next==NULL) {
	g->prev = NULL;
	p->prev = elem;
	p->next = g;
	return p;
}
    [g]         [p]
	/ \         / \
  [p]     =>  [l] [g]
  / \
[l]

При добавлении справа (два варианта):
if (p->prev==NULL && g->prev==NULL) {
	g->next = NULL;
	p->prev = g;
	p->next = elem;
	return p;
}

    [g]          [p]
	/ \          / \
      [p]  =>  [g] [r]
	  / \
	    [r]

if (p->prev==NULL && g->next==NULL) {
	g->prev = NULL;
	elem->prev = p;
	elem->next = g;
	return elem;
}

    [g]        [r]
	/ \        / \
  [p]    =>  [p] [g]
  / \
	[r]


*/
/*! \brief вставить поддерево в дерево атомарно, не нарушая структуру

	функция также используется для восстановления дерева из архива

	сохранение в архив производится функцией \ref tree_store
	порядок записи в архив позволяет восстановить дерево в том же виде без дополнительных
	операций по балансировке.
*/
//__attribute__((optimize("O2")))
tree_t* tree_insert_tree(tree_t** prev, tree_t* elem)
{
//	tree_t* node = *prev;
	tree_t *node;
	do{
    while((node=atomic_pointer_get((volatile void**)prev))!=NULL) {
        const int32_t cmp = elem->key - node->key;
		if (cmp==0){ // добавляем без изменения структуры дерева методом замены значения
                // в этом случае сбивается балансировка
			elem->value = atomic_exchange(&node->value, elem->value);
			return elem;
		}
		prev = (cmp<0)? &node->prev : &node->next;
/*        if (cmp<0) {
			// node->prev_depth++;
            if (node->prev!=NULL) {
                node=node->prev;
            } else {
                node->prev = elem;
				break;
            }
        }
		else {// if (cmp>0)
			// node->next_depth++; // этот элемент не испльзуется
            if (node->next!=NULL) {
                node=node->next;
            } else {
				node->next = elem;
				break;
            }
        }
//		path[idx++] = tree; */
    }
	//*prev = elem;
	// CAS(prev, node, elem);
	} while(!atomic_pointer_compare_and_exchange(prev, node, elem));
  /* Restore balance. This is the goodness of a non-recursive
   * implementation, when we are done with balancing we 'break'
   * the loop and we are done.
   */
	return NULL;
}

/*! \brief выполнить замену данных в узле дерева
    \param key - идентификатор узла дерева, для которого выполняется замена
    \return значение до замены
*/
void* tree_replace_data(tree_t *node, uint32_t key, void* value)
{
    while(node) {
		int32_t cmp = key - node->key;
		if(cmp==0) return atomic_exchange(&node->value, value);
        node = (cmp < 0)? node->prev: node->next;
    }
    return NULL;
}
/*! \fn tree_t* tree_splay(tree_t** root, x)

    \brief  Поднимает элемент с ключом key к вершине дерева
    \return новую вершину дерева


    реализация сплая не очень понравилась, потому что кушает дополнительную память на стеке
*/
#if 0
tree_t* tree_splay(tree_t* node, uint32_t key)
{// \todo иначе определять глубину, фактически мы берем число элементов, а не максимальную глубину дерева
    const uint16_t max_depth = node->prev_depth >= node->next_depth ? node->prev_depth : node->next_depth;
    int idx=0;
    tree_t* parent;
    tree_t* path[max_depth];
    while(node) {
		register int32_t cmp = key - node->key;
		if(cmp==0) break;
		path[idx++] = node;
        node = (cmp < 0)? node->prev: node->next;
    }
    if (node==NULL || idx==0) return node;
    parent = path[--idx];
    while (1) {
        if (parent->prev == node) {
            node = tree_rotate_right(parent);
        } else {// if (parent->next == node)
            node = tree_rotate_left(parent);
        }
        if (idx == 0) break;
        tree_t* grande = path[--idx];
        if (grande->prev == parent) {
            grande->prev = node;
            grande->prev_depth = _depth(node);
        } else {//if (grande->next == parent)
            grande->next = node;
            grande->next_depth = _depth(node);
        }
        parent = grande;
    }
    return node;
}
#endif // 0
/*! \fn tree_t* tree_split(tree_t** root, x)

Запускаем splay от элемента x и возвращаем два дерева, полученные отсечением правого или левого поддерева от корня,
в зависимости от того, содержит корень элемент больше или не больше, чем x.

для этого не нужна отдельная функция
*/

/*! \fn tree_add(tree, x)

Запускаем split(tree, x), который нам возвращает деревья tree1 и tree2, их подвешиваем к x как левое и правое поддеревья соответственно.
*/
/*! \fn tree_remove(tree, x)
Запускаем splay от x элемента и возвращаем Merge от его детей.
*/
#if 0
tree_t* tree_remove(tree_t **root, uint32_t key)
{
    tree_t *node = *root;
    node = tree_splay(node, key);// поднять вершину
    if (node!=NULL) {
        *root = tree_merge(node->prev, node->next);
        node->prev =NULL;
        node->next =NULL;
        node->prev_depth =0;
        node->next_depth =0;
    }
    return node;

}
#endif // 0
#ifdef TREE_TEST
#include "r3_slice.h"
// $ gcc r3core/r3_tree.c r3core/r3_slice.c -DTREE_TEST -o r3_tree.exe
// $ gcc r3core/r3_tree.c r3core/r3_slice.c r3core/tree_remove.c r3core/tree_init.c r3core/tree_balance.c r3core/tree_merge.c r3core/tree_notify.c r3core/tree_insert.c -DTREE_TEST -o r3_tree.exe
void tree_test(tree_t * tree, int depth)
{
	if(tree->prev) tree_test(tree->prev, depth+1);
	printf("%*s[%03d] (%d-%d)\n", depth*2, "", tree->key, tree->prev_depth, tree->next_depth);
	if(tree->next) tree_test(tree->next, depth+1);

}
#define MEX_EL 57

int main()
{
	printf("tree test\n");
	uint32_t *buffer = malloc(sizeof(tree_t)*400);
	//g_slice_init(sizeof(tree_t), buffer, 400);
	//return 0;
	tree_t* tree = g_slice_alloc(sizeof(tree_t));
	tree_init(tree, 0, tree);
	int i;
	for (i=1;i<MEX_EL; i++){
		tree_t* leaf = g_slice_alloc(sizeof(tree_t));
		tree_init(leaf, i, leaf);
		tree_insert_tree(&tree, leaf);
//		tree_insert(&tree, leaf);

		printf("%03d: %p\n", i, leaf);
	}

	printf("tree test depth:\n");
	tree_test(tree, 0);
	printf("tree test balanced:\n");
	tree_balance(&tree);printf("\n");
	tree_test(tree, 0);
	printf("tree test balanced2:\n");
	tree_balance(&tree);printf("\n");
	tree_test(tree, 0);
	printf("tree test balanced3:\n");
	tree_balance(&tree);printf("\n");
	tree_test(tree, 0);

	printf("tree test splay:20\n");
//	tree = tree_splay(tree, 20);
	tree_t * leaf;
if (0) {
	leaf = tree_remove(&tree, 20);
	if (leaf) g_slice_free1(sizeof(tree_t), leaf);
}
	leaf = tree_remove(&tree, 11);
	leaf = tree_remove(&tree, 10);
	tree_test(tree, 0);
#if 0
	printf("tree test balanced4:\n");
	tree_balance(&tree);printf("\n");
	tree_test(tree, 0);
	printf("tree test balanced5:\n");
	tree_balance(&tree);printf("\n");
	tree_test(tree, 0);
	printf("tree test balanced6:\n");
	tree_balance(&tree);printf("\n");
	tree_test(tree, 0);
	printf("tree test balanced7:\n");
	tree_balance(&tree);printf("\n");
	tree_test(tree, 0);
#endif
#if 1
	printf("tree restore:\n");
void ntf(tree_t* tree, void* user_data)
{
	uint8_t **refs = user_data;
	uint8_t *ref = *refs;
	*ref = tree->key;
	ref++;
	*refs = ref;
	printf("K=%02d ", tree->key);
	tree->prev=NULL, tree->next=NULL;
	g_slice_free1(sizeof(tree_t), tree);
}
	uint8_t refs[400];
	uint8_t *ref = refs;
	int count = MEX_EL-1;//_depth(tree);// число элементов дерева
	tree_notify(tree, ntf, &ref);
	printf("\n");

	tree = g_slice_alloc(sizeof(tree_t));
	tree_init(tree, refs[0], tree);
	for(i=1; i<count;i++) {
		printf("K=%02d ", refs[i]);
		tree_t* leaf = g_slice_alloc(sizeof(tree_t));
		tree_init(leaf, refs[i], leaf);
		tree_insert_tree(&tree, leaf);
	}
	printf("\n");
	tree_test(tree, 0);
#endif
	//	tree_test(tree, 0);
	return 0;
}
#endif

