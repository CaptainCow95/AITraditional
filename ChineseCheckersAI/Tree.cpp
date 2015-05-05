#include "Tree.h"

template <class T, class A = std::allocator<T>>
Tree<T, A>::leafiterator::leafiterator(Tree<T, A>& tree, bool begin = true) : tree(tree)
{
    if (!begin)
    {
        currentNode = tree->root;
        return;
    }

    currentNode = tree->root;
    while (currentNode->children->size() > 0)
    {
        currentNode = currentNode->children[0];
    }
}

template <class T, class A = std::allocator<T>>
typename Tree<T, A>::leafiterator& Tree<T, A>::leafiterator::operator++()
{
    // TODO: Find the next node
}