#include <iostream>
#include <map>
#include <string>
#include <stack>
#include <ctime>
#include <queue>
#include <unordered_map>
#include <vector>

using namespace std;

// User structure to store account info
// struct is like a custom datatype - learned in unit 2
struct UserAccount {
    string userID;
    string tokenName;
    double balance;
};

// std::map uses a Red-Black Tree internally.
// Provides O(log n) lookup and keeps user IDs sorted automatically.
map<string, UserAccount> balanceLedger;

// Transaction structure to store all transaction details
struct Transaction {
    string fromUser;
    string toUser;
    double amount;
    string timestamp;
};

// Stack (LIFO): the most recently pushed transaction is the first one undone.
// This directly models transaction rollback behavior.
stack<Transaction> transactionHistory;

// Trade request structure for pending trades
struct TradeRequest {
    string buyerID;
    string sellerID;
    double tokenAmount;
};

// Queue (FIFO): ensures trades are processed in submission order,
// similar to a mempool in real blockchain networks.
queue<TradeRequest> pendingTrades;

// unordered_map uses a hash table for O(1) average lookup.
// Chosen over map here because we only need fast key lookup, not sorted order.
unordered_map<string, string> publicKeyRegistry;

// pool transaction struct
struct PoolTransaction {
    string transactionID;
    double fee;
    string speed;  // "fast", "medium", or "slow"
};

vector<PoolTransaction> transactionPool;

// Adjacency list representation of the token exchange graph.
// map<token, list of (neighbor_token, risk)> pairs.
map<string, vector<pair<string, double>>> tokenGraph;

struct DataChunk {
    string chunkID;
    int size;
};

vector<DataChunk> dataChunks;

// max size of one block in KB
const int BLOCK_SIZE = 1000;


// Checks if user already exists using count() — returns 1 if found, 0 if not.
// Using count() avoids accidentally inserting a blank entry via operator[].
void addUser(string usr, string token, double amt) {
    // check if user already exists
    if (balanceLedger.count(usr) > 0) {
        cout << "Error: User ID already exists." << endl;
        return;
    }
    // create new user account and fill details
    UserAccount newUser;
    newUser.userID = usr;
    newUser.tokenName = token;
    newUser.balance = amt;
    // insert into map using id as key
    balanceLedger[usr] = newUser;
    cout << "User added successfully!" << endl;
}

// function to get the balance of a user
void getBalance(string userID) {
    // reject empty userID upfront to avoid inserting a blank key into map
    if (userID.empty()) {
        cout << "Error: User ID cannot be empty.\n";
        return;
    }
    // check if user exists
    if (balanceLedger.count(userID) == 0) {
        cout << "Error: User not found.\n";
        return;
    }
    cout << "User ID: " << balanceLedger[userID].userID << "\n";
    cout << "Token: " << balanceLedger[userID].tokenName << "\n";
    cout << "Balance: " << balanceLedger[userID].balance << "\n";
}

// display all users from the balance ledger
void displayAllUsers() {
    // first check if ledger is empty
    if (balanceLedger.empty()) {
        cout << "No users in ledger." << endl;
        return;
    }
    cout << "All Users:\n";
    for (auto& entry : balanceLedger) {
        cout << entry.second.userID << " | " << entry.second.tokenName << " | " << entry.second.balance << "\n";
    }
}

// process transaction between two users
// learned about stack in unit 3, using it here for rollback functionality
void processTransaction(string sender, string receiver, double amount) {
    // check if both users exist in ledger
    if (balanceLedger.count(sender) == 0 || balanceLedger.count(receiver) == 0) {
        cout << "Error: Invalid users.\n";
        return;
    }
    // check if sender has enough balance
    if (balanceLedger[sender].balance < amount) {
        cout << "Error: Not enough balance.\n";
        return;
    }
    // cant send to yourself
    if (sender == receiver) {
        cout << "Error: Same sender and receiver.\n";
        return;
    }

    // subtract from sender, add to receiver
    balanceLedger[sender].balance -= amount;
    balanceLedger[receiver].balance += amount;

    // get current time for timestamp
    // ctime gives string like "Mon May 06 12:30:00 2024\n"
    time_t now = time(0);
    string timeStr = ctime(&now);
    // remove the newline at end of ctime string
    if (!timeStr.empty() && timeStr[timeStr.size() - 1] == '\n') {
        timeStr = timeStr.substr(0, timeStr.size() - 1);
    }

    // store in temp transaction structure first, then push to stack
    Transaction tempTxn;
    tempTxn.fromUser = sender;
    tempTxn.toUser = receiver;
    tempTxn.amount = amount;
    tempTxn.timestamp = timeStr;
    // push to stack so we can undo later using rollback
    transactionHistory.push(tempTxn);

    cout << "Transaction successful!\n";
}

// function to undo the last transaction using stack
// stack is LIFO so top() gives us most recent transaction
void rollbackLastTransaction() {
    // check if there are any transactions to undo
    if (transactionHistory.empty()) {
        cout << "No transactions to undo.\n";
        return;
    }
    // get last transaction from top of stack
    Transaction lastTxn = transactionHistory.top();
    // give money back to sender
    balanceLedger[lastTxn.fromUser].balance += lastTxn.amount;
    // remove money from receiver
    balanceLedger[lastTxn.toUser].balance -= lastTxn.amount;
    // remove from stack
    transactionHistory.pop();
    cout << "Rollback successful!\n";
}

// add new trade request to pending queue
void submitTrade(string buyID, string sellID, double amount) {
    // validate trade parameters
    if (buyID == sellID || amount <= 0) {
        cout << "Error: Invalid trade.\n";
        return;
    }
    TradeRequest newTrade;
    newTrade.buyerID = buyID;
    newTrade.sellerID = sellID;
    newTrade.tokenAmount = amount;
    // add to queue (FIFO ordering - first submitted = first processed)
    pendingTrades.push(newTrade);
    cout << "Trade submitted to queue.\n";
}

// process the trade at front of queue
void processNextTrade() {
    // check if queue has any trades
    if (pendingTrades.empty()) {
        cout << "Queue is empty.\n";
        return;
    }
    // get first trade from queue
    TradeRequest nextTrade = pendingTrades.front();
    // process the actual transaction
    processTransaction(nextTrade.sellerID, nextTrade.buyerID, nextTrade.tokenAmount);
    // remove from queue after processing
    pendingTrades.pop();
    cout << "Next trade processed.\n";
}

// display all pending trades without removing them from queue
void viewPendingTrades() {
    // check if there are pending trades
    if (pendingTrades.empty()) {
        cout << "Queue is empty.\n";
        return;
    }
    // Using a copy of the queue for display so the original is not modified.
    // Direct iteration on a queue requires popping, which would destroy it.
    queue<TradeRequest> tempQueue = pendingTrades;
    int pos = 1;
    while (!tempQueue.empty()) {
        TradeRequest trade = tempQueue.front();
        cout << pos << ". " << trade.buyerID << " wants " << trade.tokenAmount << " from " << trade.sellerID << "\n";
        tempQueue.pop();
        pos++;
    }
}

// register a public key for digital signature verification
void registerPublicKey(string userID, string key) {
    // first check if user exists
    if (balanceLedger.count(userID) == 0 || key.empty()) {
        cout << "Error: Invalid user or key.\n";
        return;
    }
    // store key in unordered_map registry - O(1) insertion
    publicKeyRegistry[userID] = key;
    cout << "Key registered!\n";
}

// verify if provided key matches the registered key
void verifySignature(string userID, string submittedKey) {
    // check if user has registered a key at all
    if (publicKeyRegistry.count(userID) == 0) {
        cout << "Error: No key registered for user.\n";
        return;
    }
    // simple string comparison to verify
    if (publicKeyRegistry[userID] == submittedKey) {
        cout << "Signature VALID.\n";
    } else {
        cout << "Signature INVALID.\n";
    }
}

// add a transaction to the mining pool
// speed must be "fast", "medium", or "slow" - validated below
void addToPool(string txID, double fee, string spd) {
    // speed is what we sort by now, so it must not be empty
    if (txID.empty() || spd.empty()) {
        cout << "Error: Invalid pool transaction.\n";
        return;
    }
    // create new pool transaction
    PoolTransaction newTx;
    newTx.transactionID = txID;
    newTx.fee = fee;
    newTx.speed = spd;
    // push to vector
    transactionPool.push_back(newTx);
    cout << "Added to pool!\n";
}

// helper function to convert speed string to numeric value
// so we can compare speeds in bubble sort
// fast=3, medium=2, slow=1, anything else=0
// i made this separate function to keep sort function clean
int getSpeedPriority(string spd) {
    if (spd == "fast") return 3;
    else if (spd == "medium") return 2;
    else if (spd == "slow") return 1;
    // unknown speed treated as lowest priority
    return 0;
}

// Bubble Sort — O(n^2) — used to sort pool transactions by speed priority.
// Implemented manually to demonstrate sorting algorithm knowledge.
void sortPoolBySpeed() {
    // check if pool has any transactions
    if (transactionPool.empty()) {
        cout << "Pool is empty.\n";
        return;
    }
    int n = transactionPool.size();
    // outer loop for passes
    for (int i = 0; i < n - 1; i++) {
        // inner loop for comparisons in each pass
        for (int j = 0; j < n - 1 - i; j++) {
            // convert speed to number for comparison
            int priorityJ = getSpeedPriority(transactionPool[j].speed);
            int priorityJ1 = getSpeedPriority(transactionPool[j + 1].speed);
            // swap if current has lower priority than next (descending order)
            if (priorityJ < priorityJ1) {
                // manual swap using temp variable
                PoolTransaction temp = transactionPool[j];
                transactionPool[j] = transactionPool[j + 1];
                transactionPool[j + 1] = temp;
            }
        }
    }
    cout << "Pool sorted by speed.\n";
}

// display all transactions in the pool with their speed
void displayPool() {
    // check if pool is empty
    if (transactionPool.empty()) {
        cout << "Pool is empty.\n";
        return;
    }
    for (int i = 0; i < (int)transactionPool.size(); i++) {
        cout << i + 1 << ". TX: " << transactionPool[i].transactionID
             << " Speed: " << transactionPool[i].speed << "\n";
    }
}

// add exchange pair (edge) to the token graph
// graph is adjacency list - map of token to list of (neighbor, risk) pairs
void addExchangePair(string token1, string token2, double rsk) {
    // validate parameters before adding
    if (token1.empty() || token2.empty() || rsk <= 0 || token1 == token2) {
        cout << "Error: Invalid exchange pair.\n";
        return;
    }
    // add edge from token1 to token2
    pair<string, double> edge1;
    edge1.first = token2;
    edge1.second = rsk;
    tokenGraph[token1].push_back(edge1);

    // add edge from token2 to token1 (bidirectional graph)
    pair<string, double> edge2;
    edge2.first = token1;
    edge2.second = rsk;
    tokenGraph[token2].push_back(edge2);

    cout << "Exchange pair added.\n";
}

// display the entire token exchange graph
void displayGraph() {
    if (tokenGraph.empty()) {
        cout << "Graph is empty.\n";
        return;
    }
    // Print each token and its connections.
    for (auto& entry : tokenGraph) {
        cout << entry.first << " connects to:\n";
        for (int i = 0; i < (int)entry.second.size(); i++) {
            cout << "  -> " << entry.second[i].first << " (risk: " << entry.second[i].second << ")\n";
        }
    }
}

// Dijkstra's Algorithm — finds the minimum-risk path between two tokens.
// dist[node] = minimum cumulative risk to reach that node from startToken.
// prev[node] = previous node on the optimal path (used for path reconstruction).
// visited[node] = true once a node's shortest distance is finalized.
void findSafestPath(string startToken, string endToken) {
    // validate input tokens exist in graph
    if (tokenGraph.count(startToken) == 0 || tokenGraph.count(endToken) == 0 || startToken == endToken) {
        cout << "Error: Invalid tokens.\n";
        return;
    }

    map<string, double> dist;
    map<string, bool> visited;
    map<string, string> prev;

    // set all distances to large number (acting as infinity)
    map<string, vector<pair<string, double>>>::iterator it;
    for (it = tokenGraph.begin(); it != tokenGraph.end(); it++) {
        dist[it->first] = 999999.0;
        visited[it->first] = false;
        prev[it->first] = "";
    }
    // source node distance is 0
    dist[startToken] = 0.0;

    int n = tokenGraph.size();
    for (int i = 0; i < n; i++) {
        // find unvisited node with minimum distance
        string curr = "";
        double minDist = 999999.0;

        map<string, double>::iterator dIt;
        for (dIt = dist.begin(); dIt != dist.end(); dIt++) {
            if (!visited[dIt->first] && dIt->second < minDist) {
                minDist = dIt->second;
                curr = dIt->first;
            }
        }

        if (curr == "") break;  // no more reachable nodes
        visited[curr] = true;
        if (curr == endToken) break;  // reached destination, stop early

        // relax all edges from curr node
        for (int j = 0; j < (int)tokenGraph[curr].size(); j++) {
            string neighbor = tokenGraph[curr][j].first;
            double weight = tokenGraph[curr][j].second;

            if (dist[curr] + weight < dist[neighbor]) {
                dist[neighbor] = dist[curr] + weight;
                prev[neighbor] = curr;
            }
        }
    }

    // if dist still 999999 then no path exists
    if (dist[endToken] >= 999999.0) {
        cout << "No path found.\n";
        return;
    }

    // reconstruct path by backtracking through prev[] map
    vector<string> path;
    string step = endToken;
    while (step != "") {
        path.push_back(step);
        step = prev[step];
    }

    // path is reversed so print from end to beginning
    cout << "Safest path risk: " << dist[endToken] << "\nPath: ";
    for (int i = (int)path.size() - 1; i >= 0; i--) {
        cout << path[i];
        if (i != 0) cout << " -> ";
    }
    cout << "\n";
}

// add a data chunk to the list
void addDataChunk(string chunkID, int sz) {
    // size must be positive
    if (sz <= 0) {
        cout << "Error: Invalid chunk size.\n";
        return;
    }
    DataChunk chk;
    chk.chunkID = chunkID;
    chk.size = sz;
    dataChunks.push_back(chk);
    cout << "Chunk added.\n";
}

// Greedy Best-Fit algorithm for block packing.
// For each chunk, finds the open block with the least remaining space that can still fit it.
// If no block fits, a new block is opened.
// This minimizes wasted space and is a standard heuristic for the bin-packing problem.
void packBlocksGreedy() {
    if (dataChunks.empty()) {
        cout << "No chunks to pack.\n";
        return;
    }

    // struct to represent a block being built
    struct Block {
        int currentSize;       // total size used so far
        vector<string> chunkIDs; // list of chunks in this block
    };

    vector<Block> openBlocks;  // all blocks currently being filled
    int blockNum = 1;          // counter for output numbering

    for (int i = 0; i < (int)dataChunks.size(); i++) {
        // if single chunk is bigger than entire block size
        if (dataChunks[i].size > BLOCK_SIZE) {
            // flush all open blocks first before handling oversized chunk
            for (int j = 0; j < (int)openBlocks.size(); j++) {
                cout << "Block " << blockNum << " size: " << openBlocks[j].currentSize << " chunks: ";
                for (int k = 0; k < (int)openBlocks[j].chunkIDs.size(); k++) {
                    cout << openBlocks[j].chunkIDs[k] << " ";
                }
                cout << "\n";
                blockNum++;
            }
            openBlocks.clear();

            // oversized chunk gets its own dedicated block
            cout << "Block " << blockNum << " size: " << dataChunks[i].size << " chunks: " << dataChunks[i].chunkID << "\n";
            blockNum++;
            continue;
        }

        // search for best block: one with smallest leftover space that can still fit this chunk
        int bestBlockIndex = -1;
        int smallestLeftover = BLOCK_SIZE + 1; // start with value bigger than any possible leftover

        // check each existing block to find the best fit
        for (int j = 0; j < (int)openBlocks.size(); j++) {
            int remainingSpace = BLOCK_SIZE - openBlocks[j].currentSize;

            // if chunk can fit in this block
            if (remainingSpace >= dataChunks[i].size) {
                // check if this block leaves less leftover space than current best
                if (remainingSpace < smallestLeftover) {
                    smallestLeftover = remainingSpace;
                    bestBlockIndex = j;
                }
            }
        }

        // place chunk in the best-fitting block, or start a new block if none fit
        if (bestBlockIndex != -1) {
            // found a block - add chunk to it
            openBlocks[bestBlockIndex].chunkIDs.push_back(dataChunks[i].chunkID);
            openBlocks[bestBlockIndex].currentSize += dataChunks[i].size;
        } else {
            // no block could fit - open a new block
            Block newBlock;
            newBlock.currentSize = dataChunks[i].size;
            newBlock.chunkIDs.push_back(dataChunks[i].chunkID);
            openBlocks.push_back(newBlock);
        }
    }

    // print all remaining open blocks at the end
    for (int j = 0; j < (int)openBlocks.size(); j++) {
        cout << "Block " << blockNum << " size: " << openBlocks[j].currentSize << " chunks: ";
        for (int k = 0; k < (int)openBlocks[j].chunkIDs.size(); k++) {
            cout << openBlocks[j].chunkIDs[k] << " ";
        }
        cout << "\n";
        blockNum++;
    }
}

int main() {
    cout << "--- MintChain ---\n";

    // Pre-loading sample data to demonstrate all features without manual input.
    addUser("alice", "ETH", 1000);
    addUser("bob", "ETH", 500);
    addUser("charlie", "BTC", 2.5);

    // registering keys for digital signatures
    registerPublicKey("alice", "SIG_alice99");
    registerPublicKey("bob", "SIG_bob88");

    // demo: verify alice's signature (correct key vs wrong key)
    cout << "\n--- Signature Verification Demo ---\n";
    cout << "Checking alice with correct key:\n";
    verifySignature("alice", "SIG_alice99");
    cout << "Checking alice with wrong key:\n";
    verifySignature("alice", "SIG_wrongKey");
    cout << "Checking unregistered user 'charlie':\n";
    verifySignature("charlie", "anykey");
    cout << "------------------------------------\n";

    // creating exchange pairs for the token graph
    // BTC <-> ETH risk 0.05
    // ETH <-> USDT risk 0.01 (most stable)
    // BTC <-> USDT risk 0.07
    addExchangePair("BTC", "ETH", 0.05);
    addExchangePair("ETH", "USDT", 0.01);
    addExchangePair("BTC", "USDT", 0.07);

    // adding data chunks for block packing demo
    addDataChunk("C001", 400);
    addDataChunk("C002", 700);

    // processing some initial transactions for demo
    processTransaction("alice", "bob", 100);
    processTransaction("charlie", "alice", 1.0);

    int choice = -1;
    while (true) {
        cout << "\nMenu:\n";
        cout << "1. Add User\n2. Check Balance\n3. Display All Users\n";
        cout << "4. Process Txn\n5. Rollback Txn\n";
        cout << "6. Submit Trade\n7. Process Next Trade\n8. View Trades\n";
        cout << "9. Register Key\n10. Verify Signature\n";
        cout << "11. Add to Pool\n12. Sort by Speed & Display Pool\n";
        cout << "13. Add Exchange Pair\n14. Display Graph\n15. Safest Path\n";
        cout << "16. Add Chunk\n17. Pack Blocks\n0. Exit\n";
        cout << "Choice: ";

        cin >> choice;
        if (choice == 0) break;

        switch (choice) {
            case 1: {
                string id, t;
                double amt;
                cout << "ID: "; cin >> id;
                cout << "Token: "; cin >> t;
                cout << "Balance: "; cin >> amt;
                addUser(id, t, amt);
                break;
            }
            case 2: {
                string id; cout << "ID: "; cin >> id;
                getBalance(id);
                break;
            }
            case 3: displayAllUsers(); break;
            case 4: {
                string from, to;
                double amt;
                cout << "From: "; cin >> from;
                cout << "To: "; cin >> to;
                cout << "Amount: "; cin >> amt;
                processTransaction(from, to, amt);
                break;
            }
            case 5: rollbackLastTransaction(); break;
            case 6: {
                string buyer, seller;
                double amt;
                cout << "Buyer: "; cin >> buyer;
                cout << "Seller: "; cin >> seller;
                cout << "Amount: "; cin >> amt;
                submitTrade(buyer, seller, amt);
                break;
            }
            case 7: processNextTrade(); break;
            case 8: viewPendingTrades(); break;
            case 9: {
                string u, k;
                cout << "User: "; cin >> u;
                cout << "Key: "; cin >> k;
                registerPublicKey(u, k);
                break;
            }
            case 10: {
                string u, k;
                cout << "User: "; cin >> u;
                cout << "Key: "; cin >> k;
                verifySignature(u, k);
                break;
            }
            case 11: {
                string id, speed;
                double fee;
                cout << "TX ID: "; cin >> id;
                cout << "Fee: "; cin >> fee;
                // speed must be: fast / medium / slow
                cout << "Speed (fast/medium/slow): "; cin >> speed;
                addToPool(id, fee, speed);
                break;
            }
            case 12: {
                sortPoolBySpeed();
                displayPool();
                break;
            }
            case 13: {
                string t1, t2;
                double r;
                cout << "Token 1: "; cin >> t1;
                cout << "Token 2: "; cin >> t2;
                cout << "Risk: "; cin >> r;
                addExchangePair(t1, t2, r);
                break;
            }
            case 14: displayGraph(); break;
            case 15: {
                string s, e;
                cout << "Start Token: "; cin >> s;
                cout << "End Token: "; cin >> e;
                findSafestPath(s, e);
                break;
            }
            case 16: {
                string id;
                int size;
                cout << "Chunk ID: "; cin >> id;
                cout << "Size (KB): "; cin >> size;
                addDataChunk(id, size);
                break;
            }
            case 17: packBlocksGreedy(); break;
            default: cout << "Invalid choice!\n";
        }
    }
    return 0;
}