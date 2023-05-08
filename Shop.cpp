
#include "Shop.h"

void Shop::init(int num_barbers)
{
   num_barbers_ = num_barbers;
   customer_in_chair_ = new int[num_barbers_];
   in_service_ = new bool[num_barbers_];
   money_paid_ = new bool[num_barbers];

   cond_barber_paid_ = new pthread_cond_t[num_barbers_];
   cond_barber_sleeping_ = new pthread_cond_t[num_barbers_];
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   pthread_cond_init(&cond_customer_served_, NULL);

   for(int i = 0; i < num_barbers_; i++){
      pthread_cond_init(&cond_barber_paid_[i], NULL);
      pthread_cond_init(&cond_barber_sleeping_[i], NULL);
      customer_in_chair_[i] = 0;
      in_service_[i] = false;
      money_paid_[i] = false;
   }
}

Shop::~Shop()
{
   pthread_mutex_destroy(&mutex_);
   for(int i = 0; i < num_barbers_; i++){
      //pthread_cond_destroy(&cond_customers_waiting_[i]);
      //pthread_cond_destroy(&cond_customer_served_[i]);
      pthread_cond_destroy(&cond_barber_paid_[i]);
      pthread_cond_destroy(&cond_barber_sleeping_[i]);
   }

   delete[] cond_barber_paid_;
   delete[] cond_barber_sleeping_;
}

string Shop::int2string(int i)
{
   stringstream out;
   out << i;
   return out.str( );
}

void Shop::print(int person, string message)
{
   cout << ((person > 0) ? "customer[" : "barber  [" ) << abs(person) 
        << "]: " << message << endl;
}

int Shop::get_cust_drops() const
{
   return cust_drops_;
}

int Shop::visitShop(int id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_) 
   {
      print( id,"leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return -1;
   }
   
   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service
  
   if (barbers.empty() || !waiting_chairs_.empty()) 
      {
         waiting_chairs_.push(id);
         print(id, "takes a waiting chair. # waiting seats available = " 
         + int2string(max_waiting_cust_ - waiting_chairs_.size()));
         pthread_cond_wait(&cond_customers_waiting_, &mutex_);
         waiting_chairs_.pop();
      }

   int barber_id = barbers.front();
   barbers.pop();
   print(id, "moves to the service chair[" + int2string(barber_id)+ "]. # waiting seats available = " 
   + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_[barber_id] = id;
   in_service_[barber_id] = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_[barber_id]);
   //print(id, "moves to a service chair[" + int2string(barber_id) + "]");

   pthread_mutex_unlock(&mutex_);
   return barber_id;
}

void Shop::leaveShop(int id, int barber_id) 
{
   pthread_mutex_lock(&mutex_);

   // Wait for service to be completed
   print(id, "wait for barber[" + int2string(barber_id) + "] " 
   + "to be done with the hair-cut");

   while (in_service_[barber_id] == true)
   {
      pthread_cond_wait(&cond_customer_served_, &mutex_);
   }
   
   // Pay the barber and signal barber appropriately
   money_paid_[barber_id] = true;
   pthread_cond_signal(&cond_barber_paid_[barber_id]);
   print( id, "says good-bye to the barber[" + int2string(barber_id) + "]");
   pthread_mutex_unlock(&mutex_);
}

void Shop::helloCustomer(int id) 
{
   pthread_mutex_lock(&mutex_);
   barbers.push(id);
   // If no customers than barber can sleep
   if (waiting_chairs_.empty() && customer_in_chair_[id] == 0 ) 
   {
      print(id * -1, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }

   if (customer_in_chair_[id] == 0)  // check if the customer, sit down.
   {
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }

   print(id * -1, "starts a hair-cut service for customer[" 
   + int2string(customer_in_chair_[id]) + "]");
   pthread_mutex_unlock( &mutex_ );
}

void Shop::byeCustomer(int barber_id)
{
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  print(barber_id * -1, "says he's done with a hair-cut service for customer[" 
  + int2string(customer_in_chair_[barber_id]) + "]");

  in_service_[barber_id] = false;
  money_paid_[barber_id] = false;
  pthread_cond_signal(&cond_customer_served_);

  while (money_paid_[barber_id] == false)
  {
      pthread_cond_signal(&cond_customer_served_);
      pthread_cond_wait(&cond_barber_paid_[barber_id], &mutex_);
  }

  //Signal to customer to get next one
  customer_in_chair_[barber_id] = 0;
  barbers.push(barber_id);
  print(barber_id * -1, "calls in another customer");
  pthread_cond_signal( &cond_customers_waiting_);

  pthread_mutex_unlock( &mutex_ );  // unlock
}