#include <thread>
#include <iostream>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>  // Add this

typedef struct __node_t{
    int key;
    struct __node_t * next{nullptr};
    std::mutex n_lock{};
} node_t;

class List{
    node_t *head{nullptr};
    node_t *tail{nullptr};
    int size{1000};

public:
    int insert(int key){
        node_t *new_node = new node_t; 
        if (new_node == nullptr){
            return -1;
        }
        
        new_node->key = key;
        new_node->next = nullptr;
        
        // Handle first insertion
        if (tail == nullptr) {
            head = tail = new_node;
            return 0;
        }
        
        // Lock the current tail
        tail->n_lock.lock();
        tail->next = new_node;
        node_t *old_tail = tail;
        tail = new_node;
        old_tail->n_lock.unlock(); 

        return 0;
    }


    void traverse(){
	    node_t *curr = head;
	    while (curr != nullptr){
		curr->n_lock.lock();
		std::cout<< curr->key << '\n';
		curr->n_lock.unlock();
		curr = curr->next;
	   }
    }


   
    
    ~List() {
        while (head != nullptr) {
            node_t *temp = head;
            head = head->next;
            delete temp;  
        }
    }
};

int main(){
    List list{};
    list.insert(5);
    list.insert(6);

    list.traverse();
    return 0;
}
