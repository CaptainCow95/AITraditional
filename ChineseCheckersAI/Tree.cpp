//#include "Tree.h"
//
//template <class T, class A = std::allocator<T>>
//Tree<T, A>::leafiterator::leafiterator(Tree<T, A>& tree, bool begin = true) : tree(tree)
//{
//    if (!begin)
//    {
//        currentNode = tree->root;
//        return;
//    }
//
//    currentNode = tree->root;
//    while (currentNode->children->size() > 0)
//    {
//        currentNode = currentNode->children[0];
//        indexInParent.push(0);
//    }
//}
//
//template <class T, class A = std::allocator<T>>
//typename Tree<T, A>::leafiterator& Tree<T, A>::leafiterator::operator++()
//{
//    currentNode = currentNode.parent;
//    int index = indexInParent.top();
//    indexInParent.pop();
//    ++index;
//    while (index >= currentNode.children->size())
//    {
//        if (currentNode.root)
//        {
//            currentNode = tree.root;
//            return;
//        }
//
//        currentNode = currentNode.parent;
//        index = indexInParent.top();
//        indexInParent.pop();
//        ++index;
//    }
//
//    currentNode = currentNode.children[index];
//    while (currentNode.children->size() != 0)
//    {
//        currentNode = currentNode.children[0];
//        indexInParent.push(0);
//    }
//}