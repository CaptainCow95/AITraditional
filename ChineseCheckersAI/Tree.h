#pragma once

#include <stack>
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
        TreeNode() : parent(nullptr), root(true) { }
        TreeNode(T v, TreeNode* p) : parent(p), value(v), root(false) { }
        ~TreeNode() { for (auto n : children) delete n; }

        void addChild(T child) { children.push_back(new TreeNode(child, this)); }
        size_t size() { return children.size(); }
        TreeNode& operator[] (const int index) { return *children[index]; }
        TreeNode* getParent() { return parent; }
        T& getValue() { return value; }
        bool isRoot() { return root; }
        void enterLock() { lock.lock(); }
        void exitLock() { lock.unlock(); }

    private:
        std::vector<TreeNode*> children;
        TreeNode* parent;
        T value;
        bool root;
        std::mutex lock;
    };

    class leafiterator
    {
        friend class TreeNode;
    public:
        typedef typename A::difference_type difference_type;
        typedef typename A::value_type value_type;
        typedef typename std::allocator<TreeNode>::reference reference;
        typedef typename std::allocator<TreeNode>::pointer pointer;
        typedef std::forward_iterator_tag iterator_category;

        leafiterator(Tree<T, A>& tree, bool begin = true) : tree(tree), currentNode(*tree.root)
        {
            if (!begin)
            {
                return;
            }

            while (currentNode.size() > 0)
            {
                currentNode = currentNode[0];
                indexInParent.push(0);
            }
        }

        bool operator==(const leafiterator& other) const { return &tree == &other.tree; }
        bool operator!=(const leafiterator& other) const { return !(*this == other); }

        leafiterator& operator++()
        {
            currentNode = currentNode.getParent();
            size_t index = indexInParent.top();
            indexInParent.pop();
            ++index;
            while (index >= currentNode.size())
            {
                if (currentNode.isRoot())
                {
                    currentNode = *tree.root;
                    return *this;
                }

                currentNode = currentNode.getParent();
                index = indexInParent.top();
                indexInParent.pop();
                ++index;
            }

            currentNode = currentNode[index];
            while (currentNode.size() != 0)
            {
                currentNode = currentNode[0];
                indexInParent.push(0);
            }

            return *this;
        }

        reference operator*() const { return currentNode; }
        pointer operator->() const { return &currentNode; }

    private:
        Tree<T, A>& tree;
        TreeNode& currentNode;
        std::stack<int> indexInParent;
    };

    Tree() { root = new TreeNode(); }
    ~Tree() { delete root; }

    leafiterator leafsBegin() { return leafiterator(*this); }
    leafiterator leafsEnd() { return leafiterator(*this, false); }
    TreeNode* getRoot() { return root; }

private:
    TreeNode* root;
};