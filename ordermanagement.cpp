#include <bits/stdc++.h>
using namespace std;

enum class RequestType { 
	Unknown=0, New=1, Modify=2, Cancel=3 
};
enum class ResponseType { 
	Unknown=0, Accept=1, Reject=2 
};

struct OrderRequest {
    int m_symbolId;
    double m_price;
    uint64_t m_qty;
    char m_side;
    uint64_t m_orderId;
    RequestType m_type;
};

struct OrderResponse {
    uint64_t m_orderId;
    ResponseType m_responseType;
};

class OrderManagement {
private:
    int maxOrdersPerSecond = 100;
    chrono::system_clock::time_point startTime, endTime;
    map<time_t,int> orderCount;
    map<long long, OrderRequest> pendingOrders;
    queue<long long> orderQueue;
    map<long long, chrono::system_clock::time_point> sendTimes;
    mutex mtx;

public:
    // setting time intervals
    void configureTimeWindow(int sH, int eH) {
        auto now = chrono::system_clock::now();
        time_t t = chrono::system_clock::to_time_t(now);
        tm *ltm = localtime(&t);
        ltm->tm_hour = sH;
        ltm->tm_min = ltm->tm_sec = 0;
        startTime = chrono::system_clock::from_time_t(mktime(ltm));
        ltm->tm_hour = eH;
        endTime = chrono::system_clock::from_time_t(mktime(ltm));
    }

    void startSystem() {
        // start processing thread
        thread(&OrderManagement::processQueue, this).detach();
        sendLogon();
    }

    void onData(OrderRequest&& req) {
        lock_guard<mutex> lock(mtx);
        auto now = chrono::system_clock::now();
        time_t sec = chrono::system_clock::to_time_t(now);
        if (now < startTime || now >= endTime) {
            cout << "Order " << req.m_orderId << " rejected (outside window)\n";
            return;
        }

       int typeVal = (int)(req.m_type);

		if (typeVal == 1) { // RequestType::New
		    if (orderCount[sec] < maxOrdersPerSecond) {
		        send(req);
		        orderCount[sec]++;
		        sendTimes[req.m_orderId] = now;
		    } else {
		        pendingOrders[req.m_orderId] = req;
		        orderQueue.push(req.m_orderId);
		    }

		} 
		else if (typeVal == 2) { // RequestType::Modify
		    if (pendingOrders.count(req.m_orderId)) {
		        pendingOrders[req.m_orderId].m_price = req.m_price;
		        pendingOrders[req.m_orderId].m_qty = req.m_qty;
		    }
		    else cout << "INVALID REQUEST\n";
		} 
		else if (typeVal == 3) {
			if (pendingOrders.count(req.m_orderId)) {
		        pendingOrders.erase(req.m_orderId);
		    } 
		    else cout << "INVALID REQUEST\n";
		} 
		else {
		    cout << "Unknown request type for order " << req.m_orderId << "\n";
		}
		    
	}

    void processQueue() {
        while(true) {
            this_thread::sleep_for(chrono::milliseconds(100));
            lock_guard<mutex> lock(mtx);
            time_t sec = chrono::system_clock::to_time_t(chrono::system_clock::now());
            int &count = orderCount[sec];
            while (!orderQueue.empty() && count < maxOrdersPerSecond) {
                long long id = orderQueue.front(); orderQueue.pop();
                auto it = pendingOrders.find(id);
                if (it == pendingOrders.end()) continue; // For order which are deleted from queue
                OrderRequest req = it->second;
                pendingOrders.erase(it);
                send(req);
                sendTimes[id] = chrono::system_clock::now();
                count++;
            }
        }
    }
    
    string responseTypeToStr(ResponseType resp) {
    	if ((int)resp == 1) {
        	return "Accept";
    	} 
    	else if ((int)resp == 2) {
        	return "Reject";
    	} 
        return "Unknown";
	}
    void onData(OrderResponse&& resp) {
        lock_guard<mutex> lock(mtx);
        auto now = chrono::system_clock::now();
        auto it = sendTimes.find(resp.m_orderId);
        if (it != sendTimes.end()) {
            auto ms = chrono::duration_cast<chrono::milliseconds>(now - it->second).count();
            ofstream f("order_response_log.txt", ios::app);
            f << "OrderID:" << resp.m_orderId
              << ",Resp:" << responseTypeToStr(resp.m_responseType)
              << ",RTT=" << ms << "ms\n";
        }
    }

    void send(OrderRequest& r) { 
    	cout << "Send: " << r.m_orderId << "\n"; 
    	cout << "Order Send Successfully\n";
    }
    void sendLogon() { 
    	cout << "Log_In\n"; 
    }
    void sendLogout() { 
    	cout << "Log_Out\n"; 
    }
};
// For Testing(Easy for taking input)
RequestType intToRequestType(int val) {
    if (val == 1) return RequestType::New;
    else if (val == 2)  return RequestType::Modify;
    else if (val == 3) return RequestType::Cancel;
    return RequestType::Unknown;
}
int main() {
    OrderManagement om;
    // window time
    om.configureTimeWindow(0, 24);
    om.startSystem();

    // testing
    vector<OrderRequest> reqs;
    int number_of_order; cin >> number_of_order;
    for (int i = 0; i < number_of_order; ++i) {
        int symbolId, typeInt;
        double price;
        uint64_t qty, orderId;
        char side;

        cout << "\nOrder " << i + 1 << ":\n";
        cout << "Enter Symbol ID: "; cin >> symbolId;
        cout << "Enter Price: "; cin >> price;
        cout << "Enter Quantity: "; cin >> qty;
        cout << "Enter Side (B/S): "; cin >> side;
        cout << "Enter Order ID: "; cin >> orderId;
        cout << "Enter Type (1=New, 2=Modify, 3=Cancel): "; cin >> typeInt;

        RequestType type = intToRequestType(typeInt);
        reqs.push_back({symbolId, price, qty, side, orderId, type});
    }

    for (auto &r : reqs) {
        om.onData(move(r));
        this_thread::sleep_for(chrono::milliseconds(50));
    }

    this_thread::sleep_for(chrono::seconds(1));
    
    // responses
    int number_of_responses;
	cout << "\nEnter number of responses:";
	cin >> number_of_responses;

	vector<OrderResponse> resps;
	for (int i = 0; i < number_of_responses; ++i) {
    	uint64_t orderId;
    	int responseTypeInt;

    	cout << "\nResponse " << i + 1 << ":\n";
    	cout << "Enter Order ID: "; cin >> orderId;
    	cout << "Enter Response Type (1=Accept, 2=Reject): "; cin >> responseTypeInt;

    	ResponseType respType;
    	if (responseTypeInt == 1) respType = ResponseType::Accept;
    	else if (responseTypeInt == 2) respType = ResponseType::Reject;
    	else respType = ResponseType::Unknown;

    	resps.push_back({orderId, respType});
	}
    for (auto &r : resps) om.onData(std::move(r));

    // Ensure queue becomes empty
    this_thread::sleep_for(chrono::seconds(4));
    om.sendLogout();
    return 0;
}
