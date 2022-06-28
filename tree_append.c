#include "r3_tree.h"
#include "atomic.h"

/*! \brief возвращает предшествующий элемент
    \note можно использовать предыдущий элемент для последовательной нумерации
    node = tree_append(root, leaf);
    leaf->key = node->key++; // атомарно

*/
tree_t* tree_append(tree_t *node, tree_t* tree)
{
    volatile void** ptr;
    do{
        while(node->next) {
            node = node->next;
        }
        tree->key = node->key+1;
        ptr = (volatile void**)&node->next;
    } while(!atomic_pointer_compare_and_exchange(ptr, NULL, tree));
    /// \todo выполнить балансировку
    return node;
}
