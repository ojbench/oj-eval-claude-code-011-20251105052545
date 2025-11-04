#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
    struct Node {
        T data;
        Node *left;
        Node *right;
        int dist;  // null path length for leftist heap

        Node(const T &val) : data(val), left(nullptr), right(nullptr), dist(0) {}
    };

    Node *root;
    size_t curSize;
    Compare cmp;

    // Helper function to calculate distance (null path length)
    int getDist(Node *node) const {
        return node ? node->dist : -1;
    }

    // Merge two leftist heap nodes in-place for efficiency
    Node* mergeNodes(Node *h1, Node *h2) {
        if (!h1) return h2;
        if (!h2) return h1;

        // Ensure h1 has the higher priority (larger value for max-heap)
        if (cmp(h1->data, h2->data)) {
            std::swap(h1, h2);
        }

        // h1 now has higher priority, recursively merge h1->right with h2
        h1->right = mergeNodes(h1->right, h2);

        // Maintain leftist property: ensure left subtree has larger distance
        if (getDist(h1->left) < getDist(h1->right)) {
            std::swap(h1->left, h1->right);
        }

        // Update distance
        h1->dist = getDist(h1->right) + 1;

        return h1;
    }

    // Copy subtree
    Node* copyTree(Node *node) {
        if (!node) return nullptr;

        Node *newNode = nullptr;
        try {
            newNode = new Node(node->data);
            newNode->left = copyTree(node->left);
            newNode->right = copyTree(node->right);
            newNode->dist = node->dist;
        } catch (...) {
            delete newNode;
            throw;
        }
        return newNode;
    }

    // Delete subtree
    void deleteTree(Node *node) {
        if (!node) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

public:
    /**
     * @brief default constructor
     */
    priority_queue() : root(nullptr), curSize(0), cmp() {}

    /**
     * @brief copy constructor
     * @param other the priority_queue to be copied
     */
    priority_queue(const priority_queue &other) : curSize(other.curSize), cmp(other.cmp) {
        root = copyTree(other.root);
    }

    /**
     * @brief deconstructor
     */
    ~priority_queue() {
        deleteTree(root);
    }

    /**
     * @brief Assignment operator
     * @param other the priority_queue to be assigned from
     * @return a reference to this priority_queue after assignment
     */
    priority_queue &operator=(const priority_queue &other) {
        if (this == &other) return *this;

        // Create a copy first for exception safety
        Node *newRoot = copyTree(other.root);
        size_t newSize = other.curSize;
        Compare newCmp = other.cmp;

        // If copy succeeded, replace current state
        deleteTree(root);
        root = newRoot;
        curSize = newSize;
        cmp = newCmp;

        return *this;
    }

    /**
     * @brief get the top element of the priority queue.
     * @return a reference of the top element.
     * @throws container_is_empty if empty() returns true
     */
    const T & top() const {
        if (empty()) {
            throw container_is_empty();
        }
        return root->data;
    }

    /**
     * @brief push new element to the priority queue.
     * @param e the element to be pushed
     */
    void push(const T &e) {
        try {
            Node *newNode = new Node(e);

            // Use merge to insert - note this may not be fully exception-safe
            // but provides the required O(log n) complexity
            root = mergeNodes(root, newNode);
            curSize++;
        } catch (...) {
            throw runtime_error();
        }
    }

    /**
     * @brief delete the top element from the priority queue.
     * @throws container_is_empty if empty() returns true
     */
    void pop() {
        if (empty()) {
            throw container_is_empty();
        }

        try {
            Node *oldRoot = root;
            Node *leftChild = root->left;
            Node *rightChild = root->right;

            root = mergeNodes(leftChild, rightChild);
            delete oldRoot;
            curSize--;
        } catch (...) {
            throw runtime_error();
        }
    }

    /**
     * @brief return the number of elements in the priority queue.
     * @return the number of elements.
     */
    size_t size() const {
        return curSize;
    }

    /**
     * @brief check if the container is empty.
     * @return true if it is empty, false otherwise.
     */
    bool empty() const {
        return curSize == 0;
    }

    /**
     * @brief merge another priority_queue into this one.
     * The other priority_queue will be cleared after merging.
     * The complexity is at most O(logn).
     * @param other the priority_queue to be merged.
     */
    void merge(priority_queue &other) {
        if (this == &other) return;

        try {
            root = mergeNodes(root, other.root);
            curSize += other.curSize;

            // Clear other queue
            other.root = nullptr;
            other.curSize = 0;
        } catch (...) {
            throw runtime_error();
        }
    }
};

}

#endif