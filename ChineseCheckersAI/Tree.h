#pragma once
#include <vector>
#include <memory>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Iterator stuff taken from https://stackoverflow.com/questions/7758580/writing-your-own-stl-container/7759622#7759622
template <class T, class A = std::allocator<T>>
class Tree
{
public:
    typedef A allocator_type;
    typedef typename A::value_type value_type;
    typedef typename A::reference reference;
    typedef typename A::const_reference const_reference;
    typedef typename A::difference_type difference_type;
    typedef typename A::size_type size_type;

    class TreeNode
    {
    public:
        TreeNode() : children(new std::vector<std::unique_ptr<TreeNode>>()), root(true) { }
        TreeNode(T value) : children(new std::vector<std::unique_ptr<TreeNode>>()), value(value), root(false) { }
        ~TreeNode() { delete children; }

        void addChild(T child) { children->push_back(make_unique<TreeNode>(child)); }
        size_t size() { return children->size(); }
        T& operator[] (const int index) { return (*children)[index]->value; }

    private:
        std::vector<std::unique_ptr<TreeNode>>* children;
        T value;
        bool root;
    };

    class leafiterator
    {
        friend class TreeNode;
    public:
        typedef typename A::difference_type difference_type;
        typedef typename A::value_type value_type;
        typedef typename A::reference reference;
        typedef typename A::pointer pointer;
        typedef std::forward_iterator_tag iterator_category;

        leafiterator(Tree<T, A>& tree, bool begin = true);

        bool operator==(const leafiterator& other) const { return tree == other.tree; }
        bool operator!=(const leafiterator& other) const { return !(*this == other); }

        leafiterator& operator++();

        reference operator*() const { return currentNode; }
        pointer operator->() const { return currentNode; }

    private:
        Tree<T, A>& tree;
        TreeNode* currentNode;
    };

    Tree() { root = new TreeNode(); }
    ~Tree() { delete root; }

    leafiterator leafsBegin() { return leafiterator(*this); }
    leafiterator leafsEnd() { return leafiterator(*this, false); }
    TreeNode& getRoot() { return *root; }

private:
    TreeNode* root;
};