#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include <mutex>

namespace pcv {

// Forward declaration
struct Point;

template<typename T>
class MemoryPool {
public:
    explicit MemoryPool(size_t block_size = 1024);
    ~MemoryPool();
    
    // Allocate a single object
    T* allocate();
    
    // Deallocate a single object
    void deallocate(T* ptr);
    
    // Reset the pool (deallocate all)
    void reset();
    
    // Get statistics
    size_t getAllocatedCount() const { return allocated_count_; }
    size_t getCapacity() const { return blocks_.size() * block_size_; }
    size_t getMemoryUsage() const { return blocks_.size() * block_size_ * sizeof(T); }
    
private:
    struct Block {
        std::unique_ptr<T[]> memory;
        size_t used = 0;
        
        explicit Block(size_t size) : memory(std::make_unique<T[]>(size)) {}
    };
    
    size_t block_size_;
    std::vector<std::unique_ptr<Block>> blocks_;
    std::vector<T*> free_list_;
    size_t allocated_count_ = 0;
    
    mutable std::mutex mutex_; // For thread safety
    
    void allocateNewBlock();
};

// Template implementation
template<typename T>
MemoryPool<T>::MemoryPool(size_t block_size) : block_size_(block_size) {
    allocateNewBlock();
}

template<typename T>
MemoryPool<T>::~MemoryPool() {
    reset();
}

template<typename T>
T* MemoryPool<T>::allocate() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (free_list_.empty()) {
        // Check if current block has space
        auto& current_block = blocks_.back();
        if (current_block->used < block_size_) {
            T* ptr = &current_block->memory[current_block->used++];
            allocated_count_++;
            return ptr;
        } else {
            // Need new block
            allocateNewBlock();
        }
    }
    
    // Get from free list
    T* ptr = free_list_.back();
    free_list_.pop_back();
    allocated_count_++;
    return ptr;
}

template<typename T>
void MemoryPool<T>::deallocate(T* ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Call destructor
    ptr->~T();
    
    // Add to free list
    free_list_.push_back(ptr);
    allocated_count_--;
}

template<typename T>
void MemoryPool<T>::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Clear all blocks
    blocks_.clear();
    free_list_.clear();
    allocated_count_ = 0;
    
    // Allocate initial block
    allocateNewBlock();
}

template<typename T>
void MemoryPool<T>::allocateNewBlock() {
    blocks_.push_back(std::make_unique<Block>(block_size_));
}

// Specialized pool for Point structures
class PointMemoryPool {
public:
    static PointMemoryPool& getInstance() {
        static PointMemoryPool instance;
        return instance;
    }
    
    MemoryPool<Point>& getPool() { return pool_; }
    
private:
    PointMemoryPool() : pool_(4096) {} // 4K points per block
    MemoryPool<Point> pool_;
};

} // namespace pcv