#ifndef QUEUE_H
#define QUEUE_H
#include "cmsis_os.h"
#include "atomic.h"
/*! \ingroup _queue
    \{
 */

typedef struct _Queue Queue;
/*! базовый объект для построения односвязных списков и очередей
 */
typedef struct _List List_t;
/*! базовый объект для построения деревьев
 */
typedef struct _Node Node;

struct _List {
    List_t* next;
};
struct _Node {
    Node* next;       //!< следующий в списке
    Node* parent;       //!< ссылка на родительский объект
    Node* children;   //!< дочерние объекты узла дерева
};
struct _Queue {
    List_t* head;
    List_t* tail;
};

static inline void node_init(Node* node)
{
    node->next = NULL;
    node->parent = NULL;
    node->children = NULL;
}
/*! \brief добавить узел в дерево */
static inline void node_attach(Node* parent, Node* node)
{
    node->parent = parent;
    node->next = parent->children;
    parent->children = node;
}
/*! \brief удалить узел из дерева */
static inline void node_detach(Node* parent, Node* node)
{
    Node* n = parent->children;
    Node* prev=NULL;
    while (n){
        if (n == node) {
            if (prev) prev->next = n->next;
            else parent->children = n->next;
            break;
        }
        prev = n;
        n = n->next;
    }
}
static inline List_t * list_prepend(List_t* head, List_t* elem)
{
    List_t* l  = elem;
    while (l->next) l = l->next;
    l->next = head;
    return elem;
}
static inline List_t * list_append(List_t* head, List_t* elem)
{
    if (head==NULL) return elem;
    List_t* l = head;
    while (l->next) l = l->next;
    l->next = elem;
    return head;
}

static inline
void    queue_init(Queue * q){
     q->tail = NULL;
     q->head = NULL;
}
void    queue_push(Queue * q, List_t* el);
List_t* queue_pop (Queue * q);
//! \}
#endif // QUEUE_H
