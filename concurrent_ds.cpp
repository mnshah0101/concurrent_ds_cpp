#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <memory>

class ApproximateConcurrentCounter {
private:
    // Use unique_ptr to store atomic counters to avoid copy/move issues
    std::vector<std::unique_ptr<std::atomic<int>>> thread_counters;
    int num_threads;

public:
    ApproximateConcurrentCounter(int threads) : num_threads(threads) {
        thread_counters.reserve(threads); // Reserve space to avoid reallocation
        for (int i = 0; i < threads; i++) {
            thread_counters.push_back(std::make_unique<std::atomic<int>>(0));
        }
    }

    void increment(int thread_id) {
        thread_counters[thread_id]->fetch_add(1, std::memory_order_relaxed);
    }

    int get_approximate_count() const {
        int total = 0;
        for (int i = 0; i < num_threads; i++) {
            total += thread_counters[i]->load(std::memory_order_relaxed);
        }
        return total;
    }

    int get_thread_count(int thread_id) const {
        return thread_counters[thread_id]->load(std::memory_order_relaxed);
    }
};

void counter_thread(ApproximateConcurrentCounter& counter, int thread_id, int target_count) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < target_count; i++) {
        counter.increment(thread_id);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Thread " << thread_id << " completed counting to " 
              << target_count << " in " << duration.count() << " ms" << std::endl;
}

// Alternative implementation using array instead of vector
class ApproximateConcurrentCounterArray {
private:
    static const int MAX_THREADS = 16;
    std::atomic<int> thread_counters[MAX_THREADS];
    int num_threads;

public:
    ApproximateConcurrentCounterArray(int threads) : num_threads(threads) {
        if (threads > MAX_THREADS) {
            throw std::invalid_argument("Too many threads");
        }
        for (int i = 0; i < threads; i++) {
            thread_counters[i].store(0);
        }
    }

    void increment(int thread_id) {
        thread_counters[thread_id].fetch_add(1, std::memory_order_relaxed);
    }

    int get_approximate_count() const {
        int total = 0;
        for (int i = 0; i < num_threads; i++) {
            total += thread_counters[i].load(std::memory_order_relaxed);
        }
        return total;
    }

    int get_thread_count(int thread_id) const {
        return thread_counters[thread_id].load(std::memory_order_relaxed);
    }
};

void counter_thread_array(ApproximateConcurrentCounterArray& counter, int thread_id, int target_count) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < target_count; i++) {
        counter.increment(thread_id);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Thread " << thread_id << " completed counting to " 
              << target_count << " in " << duration.count() << " ms (array version)" << std::endl;
}

// Shared counter for comparison
class SharedCounter {
private:
    std::atomic<int> counter{0};

public:
    void increment() {
        counter.fetch_add(1, std::memory_order_relaxed);
    }

    int get_count() const {
        return counter.load(std::memory_order_relaxed);
    }
};

void shared_counter_thread(SharedCounter& counter, int thread_id, int target_count) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < target_count; i++) {
        counter.increment();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Thread " << thread_id << " completed counting to " 
              << target_count << " in " << duration.count() << " ms (shared counter)" << std::endl;
}

int main() {
    const int NUM_THREADS = 4;
    const int COUNT_TARGET = 1000000; // One million
    
    std::cout << "=== Approximate Counter (unique_ptr version) ===" << std::endl;
    {
        ApproximateConcurrentCounter counter(NUM_THREADS);
        std::vector<std::thread> threads;
        
        std::cout << "Starting " << NUM_THREADS << " threads, each counting to " 
                  << COUNT_TARGET << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        
        auto overall_start = std::chrono::high_resolution_clock::now();
        
        // Launch threads
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(counter_thread, std::ref(counter), i, COUNT_TARGET);
        }
        
        // Wait for all threads to complete
        for (auto& t : threads) {
            t.join();
        }
        
        auto overall_end = std::chrono::high_resolution_clock::now();
        auto overall_duration = std::chrono::duration_cast<std::chrono::milliseconds>(overall_end - overall_start);
        
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "All threads completed in " << overall_duration.count() << " ms" << std::endl;
        std::cout << "Total approximate count: " << counter.get_approximate_count() << std::endl;
        std::cout << "Expected count: " << NUM_THREADS * COUNT_TARGET << std::endl;
        
        // Show individual thread counts
        std::cout << "\nIndividual thread counts:" << std::endl;
        for (int i = 0; i < NUM_THREADS; i++) {
            std::cout << "Thread " << i << ": " << counter.get_thread_count(i) << std::endl;
        }
    }
    
    std::cout << "\n=== Approximate Counter (array version) ===" << std::endl;
    {
        ApproximateConcurrentCounterArray counter(NUM_THREADS);
        std::vector<std::thread> threads;
        
        auto overall_start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(counter_thread_array, std::ref(counter), i, COUNT_TARGET);
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto overall_end = std::chrono::high_resolution_clock::now();
        auto overall_duration = std::chrono::duration_cast<std::chrono::milliseconds>(overall_end - overall_start);
        
        std::cout << "Array version completed in " << overall_duration.count() << " ms" << std::endl;
        std::cout << "Total count: " << counter.get_approximate_count() << std::endl;
    }
    
    std::cout << "\n=== Shared Counter (for comparison) ===" << std::endl;
    {
        SharedCounter shared_counter;
        std::vector<std::thread> threads;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(shared_counter_thread, std::ref(shared_counter), i, COUNT_TARGET);
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Shared counter completed in " << duration.count() << " ms" << std::endl;
        std::cout << "Final count: " << shared_counter.get_count() << std::endl;
    }
    
    return 0;
}
