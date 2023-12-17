#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

std::atomic<bool> terminator = false;
std::atomic<bool> terminator_input = false;

struct OrderData {
    std::string clientOrderId;
    bool buyOrSell;
    int shares;
    double limit;
    bool sidecorrectness;
};

class OrderQueue {
    std::queue<OrderData> orders;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(const OrderData& order) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            orders.push(order);
        }
        cv.notify_one();  // Notify worker thread of new order
    }

    bool pop(OrderData& order) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return (terminator || !orders.empty()); });  // Wait until there's an order
        if(terminator){
            return true;
        }
        order = orders.front();
        orders.pop();
        return true;
    }

    bool isempty(){
        return orders.empty();
    }
};
