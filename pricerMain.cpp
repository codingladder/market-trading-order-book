// C++ order book pricing
// reads from "prices.in" file

#include <iostream>
#include <fstream>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

const int TARGET_SIZE = 200;
const char* PRICES_FILE = "pricer.in";

using namespace std;


// Order Class
// Order stores price data informaton for an order
class Order {  			

	string timestamp;
	string orderId;
	float price;
	int size;			// size of order
	public:
		Order(string tStamp, string id, float p, int size) {
			timestamp = tStamp;
			orderId = id;
			price = p;
			this->size = size;
}
		void setSize(int size){ this->size = size; }
		string getTimestamp(){ return timestamp; }
		string getOrderId(){ return orderId; }
		float getPrice(){ return price; }
		int getSize(){ return size; }
};


// Order Book Class
// OrderBook is a book of price orders
class OrderBook {

	int target;				// used to target bid and ask
	int totalBid;
	int totalAsk;
	int pos;				// position
	float askIncome;
	float buyIncome;
	vector<Order> bids;
	vector<Order> asks;
	
	public:
		OrderBook(int size) {
			pos = 0;
			target = size;
			totalBid = 0;
			totalAsk = 0;
			buyIncome = 0;
			askIncome = 0;
		}
	
		// add new bid order
		void addBid(Order &order) {
		
			totalBid += order.getSize();
			addOrder(bids, order);
			sellTarget(bids, order.getTimestamp(), totalBid);
		}
	    
	    // add new ask order
		void addAsk(Order &order) {
		
			totalAsk += order.getSize();
			addOrder(asks, order);
			buyTarget(asks, order.getTimestamp(), totalAsk);
		}
		
		// reduce previous order
		void reduceOrder(string &tStamp, string &orderId, int amt) {
		
			if (reduceBook(bids, orderId, amt)) {
			
				totalBid = max(0, totalBid-amt);
				sellTarget(bids, tStamp, totalBid);
			}
			else if (reduceBook(asks, orderId, amt)) {
			
				totalAsk = max(0, totalAsk-amt);
				buyTarget(asks, tStamp, totalAsk);
			}
		}
	
	private:
		void addOrder(vector<Order> &orders, Order &order) {
		
			bool added = false; // to check if order added
			for(int i = 0; i<(int)orders.size(); i++) {
			
				if (orders[i].getPrice() > order.getPrice()) {
				
					added = true;
					// insert add
					orders.insert(orders.begin()+i, order);
					return;
				}
			}
		
			if (!added)
				orders.push_back(order);
		}
	
		bool reduceBook(vector<Order> &orders, string &orderId, int &amt) {
		
			for(vector<Order>::iterator order=orders.begin(); order!=orders.end();++order) {
				if (order->getOrderId() == orderId) { // if found order
					if (order->getSize() <= amt) 	 
						orders.erase(order);          // remove order
					else
						order->setSize(order->getSize() - amt);
					return true;
				}
			}
			return false;
		}
	
		// sell target amount
		void sellTarget(vector<Order> &orders, string tStamp, int &bookTotal) {
		
			bool isAsksTarget = (bookTotal >= target);
			if (isAsksTarget) {
			
				askIncome = 0;
				int tmpSize = target;

				for(vector<Order>::iterator order=orders.end(); order!=orders.begin();) {
				
					--order;
					addPnl(order, tmpSize, askIncome);
					if (tmpSize == 0)
						break;
				}
			
				pos = -(target);
				printOut(askIncome, "S", tStamp);
				return;
			}
		
			bool pnlChange = ((askIncome>0) && (bookTotal < target));
			if (pnlChange) {
			
				askIncome = 0;
		        printOut(askIncome, "S", tStamp);
			}
		}
		
		// buy target amount
		void buyTarget(vector<Order> &orders, string tStamp, int &bookTotal) {
		
			bool isBidsTarget = (bookTotal >= target);
			if (isBidsTarget) {
			
				buyIncome = 0;
				int tmpSize = target;
			
				for(vector<Order>::iterator order=orders.begin(); order!=orders.end();++order) {
				
					addPnl(order, tmpSize, buyIncome);
					if (tmpSize == 0)
						break;
				}

				pos = target;			
				printOut(buyIncome, "S", tStamp);			
				return;
			}
		
			bool pnlChange = ((buyIncome>0) && (bookTotal < target));
			if (pnlChange) {
			
				buyIncome = 0;			
		        printOut(buyIncome, "S", tStamp);
			}
		}
		
		// display price, B or S, and timestamp
		void printOut(float price, const char* c, string &tStamp) {
		
		    cout << tStamp << " " << c << " ";
		    (price==0) ? printf("NA") : printf("%.2f\n", price);
		}
	
		// add pnl 
		int addPnl(vector<Order>::iterator &order, int &tmpSize, float &pnl) {
		
			int currentSize = order->getSize();
			int num =  ((currentSize <= tmpSize) ? currentSize : tmpSize);
			pnl += num * order->getPrice();
			tmpSize -= num;
			return num;
		}
};


// MAIN
// This program reads prices from a file 
// and parses them to create orders 
// and manages an order book
int main(int argc, const char * argv[]) {

	OrderBook orderBook(TARGET_SIZE);			
	ifstream file;
	string line;
    
	file.open(PRICES_FILE);
	if (file.is_open()) {
	
		while (!file.eof()) {
			getline(file, line);
		
			vector<string> subStr;
			// slit line looking for spaces
			split(subStr, line, boost::is_any_of(" "));
			string timestamp = subStr[0];
			string addOrReduce = subStr[1];  // "A" or "R"
			string orderId = subStr[2];
	
			if (addOrReduce == "A") { 		 // Add order
			
				string side = subStr[3];
				float price = atof(subStr[4].c_str());
				int size = atoi(subStr[5].c_str());
		
				Order order(timestamp, orderId, price, size);
				if (side == "S") {			 // to sell
					orderBook.addAsk(order);
				}
				else {
					orderBook.addBid(order); // to buy
				}
			}
			else { // Reduce order
				static int reduceAmt = atoi(subStr[3].c_str());
				orderBook.reduceOrder(timestamp, orderId, reduceAmt);
			}
		}
	}
	else cout << "File not opened.\n";
	file.close();
	
	return 0;
}
