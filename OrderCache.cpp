// Your implementation of the OrderCache...

#include <algorithm>
#include <cstring>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#include <OrderCache.h>

// add order to the cache
void OrderCache::addOrder(Order order) {
    std::lock_guard<std::mutex> lock(m_ordersMutex);

    if ( findById(order.orderId()) != m_orders.size() ) {
        throw std::runtime_error("order already exists");
    }

    if ( order.side() != "Buy" && order.side() != "Sell" ) {
        throw std::runtime_error("order side must be Buy or Sell");
    }

    m_orders.emplace_back(order);
}

// remove order with this unique order id from the cache
void OrderCache::cancelOrder(const std::string& orderId) {
    std::lock_guard<std::mutex> lock(m_ordersMutex);

    unsigned int i = findById(orderId);
    if ( i < m_orders.size() ) {
        m_orders.at(i).cancel();
    }
}

// remove all orders in the cache for this user
void OrderCache::cancelOrdersForUser(const std::string& user) {
    std::lock_guard<std::mutex> lock(m_ordersMutex);

    unsigned int i = findByUser(0, user);
    while ( i < m_orders.size() ) {
        m_orders.at(i).cancel();
        i = findByUser(++i, user);
    }
}

// remove all orders in the cache for this security with qty >= minQty
void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) {
    std::lock_guard<std::mutex> lock(m_ordersMutex);

    unsigned int i = findBySecIdWithMinimumQty(0, securityId, minQty);
    while ( i < m_orders.size() ) {
        m_orders.at(i).cancel();
        i = findBySecIdWithMinimumQty(++i, securityId, minQty);
    }
}

// return the total qty that can match for the security id
unsigned int OrderCache::getMatchingSizeForSecurity(const std::string& securityId) {
    std::lock_guard<std::mutex> lock(m_ordersMutex);

    std::vector<Order> ordersWaitingForMatch;
    unsigned int totalMatchedSize = 0;

    unsigned int current_order_i = findBySecIdWithMinimumQty(0, securityId, 0);

    // while we have orders to process
    while ( current_order_i < m_orders.size() ) {
        // working with "m_orders" copies so changes are not made into "m_orders"
        Order o_current(m_orders.at(current_order_i));

        if (o_current.status() != "active") {
            current_order_i = findBySecIdWithMinimumQty(++current_order_i, securityId, 0);
            continue;
        }

        // check if current order have a match with waiting orders
        for(auto waiting_order_iter = ordersWaitingForMatch.begin();
            waiting_order_iter < ordersWaitingForMatch.end(); waiting_order_iter++) {

            // working with pointers to "Order"s from ordersWaitingForMatch,
            // because its data will change along iterations (no worries,
            // they are already copies from "m_orders")
            Order *po_waiting = &(*waiting_order_iter);

            if (po_waiting->status() != "active" ||
                po_waiting->side() == o_current.side() ||
                po_waiting->company() == o_current.company()) {
                continue;
            }

            ////// it's a match!

            unsigned int thisMatchSize = std::min(po_waiting->qtyLeft(), o_current.qtyLeft());
            po_waiting->increaseQtyMatched(thisMatchSize);
            o_current.increaseQtyMatched(thisMatchSize);
            totalMatchedSize += thisMatchSize;

            // if the current order is fully matched, we can stop looking for matches
            if (o_current.qtyLeft() == 0) {
                break;
            }
        }

        if (o_current.qtyLeft() > 0) {
            ordersWaitingForMatch.emplace_back(o_current);
        }

        current_order_i = findBySecIdWithMinimumQty(++current_order_i, securityId, 0);
    }

    return totalMatchedSize;
}

// return all orders in cache in a vector
std::vector<Order> OrderCache::getAllOrders() const {
    // since it's a const, we don't need the lock
    return m_orders;
}



///////////////////// PRIVATES

unsigned int OrderCache::findById(const std::string& orderId) {
    unsigned int i = 0;
    for (; i < m_orders.size(); i++ ) {
        if ( m_orders.at(i).orderId() == orderId ) {
            break;
        }
    }

    return i;
}

unsigned int OrderCache::findByUser(unsigned int startFrom, const std::string& user) {
    unsigned int i = startFrom;
    for (; i < m_orders.size(); i++ ) {
        if ( m_orders.at(i).user() == user ) {
            break;
        }
    }

    return i;
}

unsigned int OrderCache::findBySecIdWithMinimumQty(unsigned int startFrom, const std::string& securityId, unsigned int minQty) {
    unsigned int i = startFrom;
    for (; i < m_orders.size(); i++ ) {
        if ( m_orders.at(i).securityId() == securityId && m_orders.at(i).qty() >= minQty ) {
            break;
        }
    }

    return i;
}
