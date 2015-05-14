#pragma once

#include <vector>
#include <memory>

class MoveTree
{
public:
    class MoveTreeNode
    {
    public:
        MoveTreeNode() : samples(0), payout(0), children(nullptr), parent(nullptr), root(true), size(0), addingChildren(false) { }
        MoveTreeNode(int samples, int64_t payout, Move move, MoveTreeNode* p) : samples(samples), payout(payout), children(nullptr), parent(p), move(move), root(false), size(0), addingChildren(false) { }
        ~MoveTreeNode()
        {
            if (children != nullptr)
            {
                for (auto n : *children) delete n;
                delete children;
            }
        }

        void addChildren(std::vector<MoveTreeNode*>* children)
        {
#ifdef DEBUG
            assert(this->children == nullptr);
#endif
            this->children = children;
            size += children->size();
        }

        MoveTreeNode* get(const int index)
        {
            return (*children)[index];
        }

        size_t getSize() { return size; }
        MoveTreeNode* getParent() { return parent; }
        Move getMove() { return move; }
        bool isRoot() { return root; }
        void enterLock() { lock.lock(); }
        void exitLock() { lock.unlock(); }

        std::atomic<int32_t> samples;
        std::atomic<int64_t> payout;
        std::atomic<bool> addingChildren;

    private:
        std::vector<MoveTreeNode*>* children;
        MoveTreeNode* parent;
        Move move;
        bool root;
        std::atomic<size_t> size;
        std::mutex lock;
    };

    MoveTree() { root = new MoveTreeNode(); }
    ~MoveTree() { delete root; }

    MoveTreeNode* getRoot() { return root; }

private:
    MoveTreeNode* root;
};