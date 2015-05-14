#pragma once

#include <vector>
#include <memory>

class MoveTree
{
public:
    class MoveTreeNode
    {
    public:
        MoveTreeNode() : samples(0), payout(0), addingChildren(false), children(nullptr), parent(nullptr), root(true), size(0) { }
        MoveTreeNode(int samples, int64_t payout, float ucbValue, Move move, MoveTreeNode* p) : samples(samples), payout(payout), ucbValue(ucbValue), addingChildren(false), children(nullptr), parent(p), move(move), root(false), size(0) { }
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
        float ucbValue;
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