#include <set>
#include <iostream>

class Order;

class Limit {
public:
    double limitPrice;
    Order* headOrder = nullptr;
    Order* tailOrder = nullptr;

    Limit(double price) : limitPrice(price) {}

    bool operator<(const Limit& other) const {
        return limitPrice < other.limitPrice;
    }

    void addOrder(Order* order);
    void removeOrder(Order* order);
};

class Order {
public:
    static int nextId;

    int id = ++nextId;
    std::string clientOrderId;
    bool buyOrSell;
    int shares;
    double limit;
    Order* nextOrder = nullptr;
    Order* prevOrder = nullptr;
    Limit* parentLimit = nullptr;

    Order(std::string clientOrderId, bool buyOrSell, int shares, double limit)
            : clientOrderId(clientOrderId), buyOrSell(buyOrSell), shares(shares), limit(limit) {}
};

int Order::nextId = 0;

void Limit::addOrder(Order* order) {
    if (!headOrder) {
        headOrder = tailOrder = order;
    } else {
        tailOrder->nextOrder = order;
        order->prevOrder = tailOrder;
        tailOrder = order;
    }
    order->parentLimit = this;
}

void Limit::removeOrder(Order* order) {
    if (order->prevOrder) {
        order->prevOrder->nextOrder = order->nextOrder;
    } else {
        headOrder = order->nextOrder;
    }
    if (order->nextOrder) {
        order->nextOrder->prevOrder = order->prevOrder;
    } else {
        tailOrder = order->prevOrder;
    }
}

class OrderBook {
private:
    std::set<Limit> buys;
    std::set<Limit> sells;

    Limit* findOrCreateLimit(double price, bool buyOrSell);

public:
    std::string instrument;
    std::string addOrder(std::string clientOrderId, bool buyOrSell, int shares, double limit, bool sidecorrectness);
    std::string match(Order& order);
    // Other functions...
};

Limit* OrderBook::findOrCreateLimit(double price, bool buyOrSell) {
    Limit tempLimit(price);
    auto& targetSet = buyOrSell ? buys : sells;
    auto it = targetSet.find(tempLimit);
    if (it == targetSet.end()) {
        it = targetSet.emplace(tempLimit).first;
    }
    return const_cast<Limit*>(&(*it)); // unsafe but required due to set's element immutability
}

std::string OrderBook::addOrder(std::string clientOrderId, bool buyOrSell, int shares, double limit, bool sidecorrectness) {
    std::string messages;

    // Validate shares
    if (shares <= 0) {
        messages += clientOrderId + "," + instrument + "," +
                    std::to_string(buyOrSell) + "," + "rejected,InvalidShares," + std::to_string(shares) + "," + std::to_string(limit) + "\n";
        return messages;
    }

    // Validate limit price
    if (limit <= 0) {
        messages += clientOrderId + "," + instrument + "," +
                    std::to_string(buyOrSell) + "," + "rejected,InvalidLimit " + std::to_string(shares) + "," + std::to_string(limit) + "\n";

        return messages;
    }

    if (!sidecorrectness){
        messages += clientOrderId + "," + instrument + "," +
                    std::to_string(buyOrSell) + "," + "rejected,InvalidSide," + std::to_string(shares) + "," + std::to_string(limit) + "\n";

    }


    Order* order = new Order(clientOrderId, buyOrSell, shares, limit);
    messages += match(*order);
    if (order->shares > 0) {
        Limit* appropriateLimit = findOrCreateLimit(limit, buyOrSell);
        appropriateLimit->addOrder(order);
        messages += order->clientOrderId + "," +std::to_string(order->id) + "," + instrument + "," +
                    std::to_string(order->buyOrSell) + "," + "New," + std::to_string(order->shares) + "," + std::to_string(limit) + "\n";
    } else {
        delete order;
    }

    return messages;
}

std::string OrderBook::match(Order& order) {
    std::string messages;
    std::string common_component = order.clientOrderId + "," +std::to_string(order.id) + "," + instrument + "," +
                                   std::to_string(order.buyOrSell) + ",";
    auto &oppositeSet = order.buyOrSell ? sells : buys;

    while (order.shares > 0 && !oppositeSet.empty()) {
        Limit &bestLimit = const_cast<Limit &>(order.buyOrSell ? *oppositeSet.begin() : *oppositeSet.rbegin());
        if (!((bestLimit.limitPrice <= order.limit && order.buyOrSell) ||
            (bestLimit.limitPrice >= order.limit && !order.buyOrSell))) {
            return messages;
        }
        while (order.shares > 0 && bestLimit.headOrder) {
            Order *oppositeOrder = bestLimit.headOrder;
            int matchedShares = std::min(order.shares, oppositeOrder->shares);

            order.shares -= matchedShares;
            oppositeOrder->shares -= matchedShares;

            if (oppositeOrder->shares == 0) {
                bestLimit.removeOrder(oppositeOrder);
                messages += oppositeOrder->clientOrderId + "," +std::to_string(oppositeOrder->id) + "," + instrument + "," +
                            std::to_string(oppositeOrder->buyOrSell) + "," + "Fill," +
                            std::to_string(matchedShares) + "," + std::to_string(bestLimit.limitPrice) + "\n";

                if (order.shares == 0) {
                    messages += common_component + "Fill," + std::to_string(matchedShares) + "," +
                                std::to_string(bestLimit.limitPrice) + "\n";
                } else {
                    messages += common_component + "PFill," + std::to_string(matchedShares) + "," +
                                std::to_string(bestLimit.limitPrice) + "\n";
                }
                delete oppositeOrder;
            } else {
                messages += oppositeOrder->clientOrderId + ","  +std::to_string(oppositeOrder->id) + "," + instrument + "," +
                            std::to_string(oppositeOrder->buyOrSell) + "," + "PFill," +
                            std::to_string(matchedShares) + "," + std::to_string(bestLimit.limitPrice) + "\n";
                messages += common_component + "Fill," + std::to_string(matchedShares) + "," +
                            std::to_string(bestLimit.limitPrice) + "\n";
            }
        }
        if (!bestLimit.headOrder) {
            oppositeSet.erase(bestLimit);
        }
    }

    return messages;

}