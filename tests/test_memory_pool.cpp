#include <gtest/gtest.h>
#include "core/MemoryPool.h"
#include "core/PointCloud.h"
#include <vector>
#include <thread>

using namespace pcv;

struct TestObject {
    int value;
    float data[10];
    
    TestObject() : value(0) {
        for (int i = 0; i < 10; ++i) {
            data[i] = 0.0f;
        }
    }
    
    explicit TestObject(int v) : value(v) {
        for (int i = 0; i < 10; ++i) {
            data[i] = static_cast<float>(v + i);
        }
    }
};

TEST(MemoryPoolTest, BasicAllocation) {
    MemoryPool<TestObject> pool(100);
    
    // Allocate objects
    std::vector<TestObject*> objects;
    for (int i = 0; i < 50; ++i) {
        TestObject* obj = pool.allocate();
        ASSERT_NE(obj, nullptr);
        new (obj) TestObject(i); // Placement new
        objects.push_back(obj);
    }
    
    EXPECT_EQ(pool.getAllocatedCount(), 50);
    
    // Verify objects
    for (int i = 0; i < 50; ++i) {
        EXPECT_EQ(objects[i]->value, i);
    }
    
    // Deallocate half
    for (int i = 0; i < 25; ++i) {
        pool.deallocate(objects[i]);
    }
    
    EXPECT_EQ(pool.getAllocatedCount(), 25);
}

TEST(MemoryPoolTest, ReuseMemory) {
    MemoryPool<TestObject> pool(10);
    
    // Allocate and deallocate
    TestObject* obj1 = pool.allocate();
    new (obj1) TestObject(42);
    pool.deallocate(obj1);
    
    // Allocate again - should reuse memory
    TestObject* obj2 = pool.allocate();
    EXPECT_EQ(obj1, obj2); // Should get same memory back
}

TEST(MemoryPoolTest, MultipleBlocks) {
    MemoryPool<TestObject> pool(10); // Small block size
    
    // Allocate more than one block
    std::vector<TestObject*> objects;
    for (int i = 0; i < 25; ++i) {
        TestObject* obj = pool.allocate();
        ASSERT_NE(obj, nullptr);
        objects.push_back(obj);
    }
    
    EXPECT_EQ(pool.getAllocatedCount(), 25);
    EXPECT_GE(pool.getCapacity(), 25); // Should have allocated multiple blocks
}

TEST(MemoryPoolTest, Reset) {
    MemoryPool<TestObject> pool(100);
    
    // Allocate some objects
    for (int i = 0; i < 50; ++i) {
        pool.allocate();
    }
    
    EXPECT_EQ(pool.getAllocatedCount(), 50);
    
    // Reset pool
    pool.reset();
    
    EXPECT_EQ(pool.getAllocatedCount(), 0);
    EXPECT_GT(pool.getCapacity(), 0); // Should still have capacity
}

TEST(MemoryPoolTest, ThreadSafety) {
    MemoryPool<TestObject> pool(1000);
    const int num_threads = 4;
    const int allocations_per_thread = 100;
    
    std::vector<std::thread> threads;
    std::vector<std::vector<TestObject*>> thread_objects(num_threads);
    
    // Spawn threads that allocate and deallocate
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&pool, &thread_objects, t, allocations_per_thread]() {
            for (int i = 0; i < allocations_per_thread; ++i) {
                TestObject* obj = pool.allocate();
                ASSERT_NE(obj, nullptr);
                thread_objects[t].push_back(obj);
            }
            
            // Deallocate half
            for (int i = 0; i < allocations_per_thread / 2; ++i) {
                pool.deallocate(thread_objects[t][i]);
            }
        });
    }
    
    // Wait for threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Check final count
    EXPECT_EQ(pool.getAllocatedCount(), 
              num_threads * allocations_per_thread / 2);
}

TEST(MemoryPoolTest, PointMemoryPool) {
    auto& pool = PointMemoryPool::getInstance().getPool();
    
    // Should be able to allocate points
    Point* p1 = pool.allocate();
    ASSERT_NE(p1, nullptr);
    new (p1) Point(glm::vec3(1.0f, 2.0f, 3.0f));
    
    EXPECT_EQ(p1->position.x, 1.0f);
    EXPECT_EQ(p1->position.y, 2.0f);
    EXPECT_EQ(p1->position.z, 3.0f);
    
    pool.deallocate(p1);
}

TEST(MemoryPoolTest, MemoryUsage) {
    MemoryPool<TestObject> pool(1000);
    
    size_t initial_usage = pool.getMemoryUsage();
    EXPECT_GT(initial_usage, 0);
    
    // Allocate more to trigger new block
    for (int i = 0; i < 1500; ++i) {
        pool.allocate();
    }
    
    size_t after_usage = pool.getMemoryUsage();
    EXPECT_GT(after_usage, initial_usage);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}