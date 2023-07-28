#pragma once

#include <mutex>
#include <string>
#include <vector>

class Order
{

 public:

  // do not alter signature of this constructor
  Order(
      const std::string& ordId,
      const std::string& secId,
      const std::string& side,
      const unsigned int qty,
      const std::string& user,
      const std::string& company)
      : m_orderId(ordId),
        m_securityId(secId),
        m_side(side),
        m_qty(qty),
        m_user(user),
        m_company(company),
        m_cancelled(false),
        m_qtyMatched(0) { }

  // do not alter these accessor methods
  std::string orderId() const    { return m_orderId; }
  std::string securityId() const { return m_securityId; }
  std::string side() const       { return m_side; }
  std::string user() const       { return m_user; }
  std::string company() const    { return m_company; }
  unsigned int qty() const       { return m_qty; }
  unsigned int qtyLeft()         { return m_qty - m_qtyMatched; }
  std::string status() const     {
      if ( m_qty == m_qtyMatched ) {
          return "fulfilled";
      } else if ( m_cancelled ) {
          return "cancelled";
      } else {
          return "active";
      }
  }

  // methods changing attributes contents (need to be
  // aware of these on multithreading)
  void cancel() { m_cancelled = "cancelled"; };
  void increaseQtyMatched(unsigned int qty) {
      if ( status() != "active" ) {
          throw std::runtime_error("qty increase can only be made on a active order");
      }

      if (m_qtyMatched + qty > m_qty) {
          throw std::runtime_error("cannot match order qty beyond its starting qty");
      }

      m_qtyMatched += qty;
  };

 private:

  // use the below to hold the order data
  // do not remove the these member variables
  std::string m_orderId;     // unique order id
  std::string m_securityId;  // security identifier
  std::string m_side;        // side of the order, eg Buy or Sell
  unsigned int m_qty;        // qty for this order
  std::string m_user;        // user name who owns this order
  std::string m_company;     // company for user
  bool m_cancelled;          // true if order was cancelled
  unsigned int m_qtyMatched; // qty already matched (updated along execution)
};


// Provide an implementation for the OrderCacheInterface interface class.
// Your implementation class should hold all relevant data structures you think
// are needed.
class OrderCacheInterface
{

 public:

  // implement the 6 methods below, do not alter signatures

  // add order to the cache
  virtual void addOrder(Order order) = 0;

  // remove order with this unique order id from the cache
  virtual void cancelOrder(const std::string& orderId) = 0;

  // remove all orders in the cache for this user
  virtual void cancelOrdersForUser(const std::string& user) = 0;

  // remove all orders in the cache for this security with qty >= minQty
  virtual void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) = 0;

  // return the total qty that can match for the security id
  virtual unsigned int getMatchingSizeForSecurity(const std::string& securityId) = 0;

  // return all orders in cache in a vector
  virtual std::vector<Order> getAllOrders() const = 0;

};

class OrderCache : public OrderCacheInterface {
private:
    std::vector<Order> m_orders;
    std::mutex m_ordersMutex;

    unsigned int findById(const std::string& orderId);
    unsigned int findByUser(unsigned int startFrom, const std::string& user);
    unsigned int findBySecIdWithMinimumQty(unsigned int startFrom, const std::string& securityId, unsigned int minQty);

public:
    void addOrder(Order order);
    void cancelOrder(const std::string& orderId);
    void cancelOrdersForUser(const std::string& user);
    void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty);
    unsigned int getMatchingSizeForSecurity(const std::string& securityId);
    std::vector<Order> getAllOrders() const;
};
