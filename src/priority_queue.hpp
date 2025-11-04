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

    // Merge two leftist heap nodes with exception safety
    Node* mergeNodes(Node *h1, Node *h2) {
        if (!h1) return copyTree(h2);
        if (!h2) return copyTree(h1);

        // Ensure h1 has the higher priority (larger value for max-heap)
        if (cmp(h1->data, h2->data)) {
            std::swap(h1, h2);
        }

        Node *newNode = nullptr;
        try {
            newNode = new Node(h1->data);

            // Recursively merge h1->right with h2
            Node *mergedRight = mergeNodes(h1->right, h2);

            // Copy left subtree
            Node *copiedLeft = copyTree(h1->left);

            newNode->right = mergedRight;
            newNode->left = copiedLeft;

            // Maintain leftist property: ensure left subtree has larger distance
            if (getDist(newNode->left) < getDist(newNode->right)) {
                std::swap(newNode->left, newNode->right);
            }

            // Update distance
            newNode->dist = getDist(newNode->right) + 1;

        } catch (...) {
            delete newNode;
            throw;
        }

        return newNode;
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
        // Save current state for exception safety
        Node *oldRoot = root;
        size_t oldSize = curSize;

        try {
            Node *newNode = new Node(e);
            Node *newRoot = mergeNodes(root, newNode);

            // Delete old tree and update root
            deleteTree(root);
            root = newRoot;
            curSize++;
        } catch (...) {
            // Restore original state if any exception occurs
            root = oldRoot;
            curSize = oldSize;
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

        // Save current state for exception safety
        Node *oldRoot = root;
        size_t oldSize = curSize;

        try {
            Node *leftChild = copyTree(root->left);
            Node *rightChild = copyTree(root->right);

            // Merge children to form new root
            Node *newRoot = mergeNodes(leftChild, rightChild);

            // Delete old tree and update root
            deleteTree(root);
            root = newRoot;
            curSize--;
        } catch (...) {
            // Restore original state if any exception occurs
            root = oldRoot;
            curSize = oldSize;
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

        // Save current states for exception safety
        Node *oldRoot1 = root;
        Node *oldRoot2 = other.root;
        size_t oldSize1 = curSize;
        size_t oldSize2 = other.curSize;

        try {
            // Create copies of both subtrees
            Node *copiedRoot1 = copyTree(root);
            Node *copiedRoot2 = copyTree(other.root);

            // Merge the copies
            Node *newRoot = mergeNodes(copiedRoot1, copiedRoot2);

            // Delete old trees and update roots
            deleteTree(root);
            deleteTree(other.root);
            root = newRoot;
            curSize += other.curSize;

            // Clear other queue
            other.root = nullptr;
            other.curSize = 0;
        } catch (...) {
            // Restore original states if any exception occurs
            root = oldRoot1;
            other.root = oldRoot2;
            curSize = oldSize1;
            other.curSize = oldSize2;
            throw runtime_error();
        }
    }
};

}

#endif