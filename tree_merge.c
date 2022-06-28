#include "r3_tree.h"
/*! \fn tree_t* tree_merge(tree_t* tree1, tree_t *tree2)
    \brief

У нас есть два дерева tree1 и tree2, причём подразумевается, что все элементы первого дерева меньше элементов второго.
Запускаем splay от самого большого элемента в дереве tree1 (пусть это элемент i). После этого корень tree1 содержит элемент i,
при этом у него нет правого ребёнка. Делаем tree2 правым поддеревом i и возвращаем полученное дерево.

*/
tree_t* tree_merge(tree_t *tree1, tree_t *tree2)
{
    if (tree1==NULL) return tree2;
    if (tree2==NULL) return tree1;
    if (tree1->next_depth < tree2->prev_depth) {
        while (tree1->next) {// поднимаем самый правый элемент на вершину
            tree1 = tree_rotate_left(tree1);
        }
        tree1->next = tree2;// цепляем второе дерево справа
        tree1->next_depth = _depth(tree2);
        return tree1;
    }
    else {
        while (tree2->prev) {// поднимаем самый левый элемент на вершину
            tree2 = tree_rotate_right(tree2);
        }
        tree2->prev = tree1;// цепляем вервое дерево слева
        tree2->prev_depth = _depth(tree1);//глубина по дереву изменилась!
        return tree2;
    }
}
