//Brandon St Pierre
//4/26/2023
#ifndef SHOP_H_
#define SHOP_H_

#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>

using namespace std;

#define kDefaultNumChairs 3
#define kDefaultBarbers 1

class Shop
{
   public:

      Shop(int num_barbers, int num_chairs) : max_waiting_cust_((num_chairs > 0 ) ? num_chairs :
      kDefaultNumChairs), cust_drops_(0)
      {
         init(num_barbers);
      };

      Shop() : max_waiting_cust_(kDefaultNumChairs), cust_drops_(0)
      {
         init(1);
      };
      
      ~Shop();
      
      int visitShop(int id); // return true only when a customer got a service
      void leaveShop(int customer_id, int barber_id);
      void helloCustomer(int id);
      void byeCustomer(int id);
      int get_cust_drops() const;

   private:

      int* customer_in_chair_;
      bool* in_service_;
      bool* money_paid_;
      queue<int> barbers;
      const int max_waiting_cust_; // the max number of threads that can wait
      queue<int> waiting_chairs_; // includes the ids of all waiting threads
      int cust_drops_;
      int num_barbers_;
      
      // Mutexes and condition variables to coordinate threads
      // mutex_ is used in conjuction with all conditional variables
      pthread_mutex_t mutex_;
      pthread_cond_t cond_customers_waiting_;
      pthread_cond_t cond_customer_served_;
      pthread_cond_t* cond_barber_paid_;
      pthread_cond_t* cond_barber_sleeping_;

      void init(int num_barber);
      string int2string(int i);
      void print(int person, string message);
};
#endif
