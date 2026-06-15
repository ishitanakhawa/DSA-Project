# MintChain: Virtual Cryptographic Token Mint Ledger

**Course:** Data Structures & Algorithms with C++ — Semester II  
**Institution:** ITM Skills University  
**Problem No.:** 54

---

## Project Overview

MintChain is a console-based C++ system that simulates a cryptocurrency token ledger. It records token ownership, processes trades in order, verifies digital signatures, finds optimal exchange routes, and packs transaction data into blocks efficiently.

The project demonstrates the practical application of seven core data structures and two algorithms, each chosen to solve a specific real-world problem modelled after platforms like Ethereum and Solana.

---

## Problems Solved

| Legacy Problem | Solution Implemented |
|---|---|
| Balance ledger too large for fast in-memory storage | `std::map` with O(log n) sorted lookups |
| Incorrect transactions cannot be reversed | `std::stack` for LIFO rollback |
| Trades processed randomly, not in order | `std::queue` for strict FIFO processing |
| Signature verification scans entire key registry | `std::unordered_map` for O(1) average lookup |
| No way to rank transactions by urgency | `std::vector` + manual Bubble Sort |
| No map of token exchange relationships | Adjacency list graph (`map<string, vector<pair>>`) |
| Users miss cheaper multi-step conversion routes | Dijkstra's Algorithm for minimum-risk pathfinding |
| Transaction data wastes blockchain space | Greedy Best-Fit bin-packing algorithm |

---

## Data Structures & Algorithms Used

### 1. `std::map<string, UserAccount>` — Balance Ledger
- **Why:** Internally a Red-Black Tree. Provides O(log n) insert, lookup, and delete. Keeps user accounts automatically sorted by ID.
- **Used in:** `addUser()`, `getBalance()`, `displayAllUsers()`, `processTransaction()`

### 2. `std::stack<Transaction>` — Transaction Rollback
- **Why:** LIFO (Last In, First Out) structure. The most recently executed transaction sits at the top and can be undone in O(1).
- **Used in:** `processTransaction()` (push), `rollbackLastTransaction()` (top + pop)

### 3. `std::queue<TradeRequest>` — Pending Trade Queue
- **Why:** FIFO (First In, First Out) structure. Guarantees trades are processed in the exact order they were submitted, mirroring a real blockchain mempool.
- **Used in:** `submitTrade()` (push), `processNextTrade()` (front + pop), `viewPendingTrades()` (copy + iterate)

### 4. `std::unordered_map<string, string>` — Public Key Registry
- **Why:** Hash table with O(1) average lookup. Eliminates the need to scan the entire key registry for signature verification.
- **Used in:** `registerPublicKey()`, `verifySignature()`

### 5. `std::vector<PoolTransaction>` + Bubble Sort — Transaction Pool
- **Why:** Dynamic array allowing random-access indexing, required for in-place sorting. Bubble Sort is implemented manually to demonstrate the algorithm.
- **Time Complexity:** Bubble Sort is O(n²). A helper `getSpeedPriority()` converts speed strings (fast/medium/slow) to integers (3/2/1) for comparison.
- **Used in:** `addToPool()`, `sortPoolBySpeed()`, `displayPool()`

### 6. Adjacency List (`map<string, vector<pair<string, double>>>`) — Token Exchange Graph
- **Why:** Memory-efficient graph representation for sparse networks. Each token maps to a list of (neighbor token, risk) pairs. Bidirectional edges model symmetric exchange relationships.
- **Used in:** `addExchangePair()`, `displayGraph()`, `findSafestPath()`

### 7. Dijkstra's Algorithm — Safest Exchange Path
- **Why:** Standard greedy algorithm for single-source shortest path on non-negative weighted graphs. Here "shortest" means lowest cumulative risk.
- **Time Complexity:** O(V²) with linear scan for minimum distance. Path is reconstructed by backtracking through a `prev[]` map.
- **Used in:** `findSafestPath()`

### 8. Greedy Best-Fit — Block Packing
- **Why:** Heuristic for the NP-hard bin-packing problem. For each chunk, finds the open block with the least remaining space that can still fit it, minimizing wasted space.
- **Used in:** `packBlocksGreedy()`

---

## How to Compile and Run

Requires a standard C++ compiler (`g++`). No external libraries needed.

**1. Compile:**
```bash
g++ main.cpp -o main -std=c++11
```

**2. Run:**
```bash
# Mac / Linux
./main

# Windows
main.exe
```

---

## Features & Menu Options

The program starts with a preloaded demo (3 users, 2 registered keys, 3 exchange pairs, 2 data chunks, 2 transactions) so all features can be tested immediately.

### User Management (1–3)
| Option | Feature | Description |
|---|---|---|
| 1 | Add User | Create a new account with user ID, token name, and initial balance |
| 2 | Check Balance | Look up a specific user's account details |
| 3 | Display All Users | View all users sorted by ID (automatic with `std::map`) |

### Transaction Management (4–5)
| Option | Feature | Description |
|---|---|---|
| 4 | Process Transaction | Transfer tokens between two users with balance validation |
| 5 | Rollback Transaction | Undo the most recent transaction (LIFO via stack) |

### Trade Queue (6–8)
| Option | Feature | Description |
|---|---|---|
| 6 | Submit Trade | Add a trade request to the pending queue (FIFO) |
| 7 | Process Next Trade | Execute the trade at the front of the queue |
| 8 | View Pending Trades | Display all queued trades without removing them |

### Digital Signatures (9–10)
| Option | Feature | Description |
|---|---|---|
| 9 | Register Key | Store a public key for a user in the hash registry |
| 10 | Verify Signature | Check a submitted key against the registered public key |

### Transaction Pool & Mining (11–12)
| Option | Feature | Description |
|---|---|---|
| 11 | Add to Pool | Add a transaction with a fee and speed (fast/medium/slow) |
| 12 | Sort & Display Pool | Sort by speed priority using Bubble Sort, then display |

### Token Graph & Exchange Paths (13–15)
| Option | Feature | Description |
|---|---|---|
| 13 | Add Exchange Pair | Add a bidirectional edge between two tokens with a risk value |
| 14 | Display Graph | Print the full token exchange adjacency list |
| 15 | Safest Path | Run Dijkstra's algorithm to find the lowest-risk exchange route |

### Block Packing (16–17)
| Option | Feature | Description |
|---|---|---|
| 16 | Add Chunk | Add a data chunk (in KB) to the packing list |
| 17 | Pack Blocks | Run Greedy Best-Fit to pack chunks into 1000 KB blocks |

---

## Preloaded Demo Data

On startup, the program automatically loads the following data:

**Users:**
| User ID | Token | Balance |
|---|---|---|
| alice | ETH | 1000 |
| bob | ETH | 500 |
| charlie | BTC | 2.5 |

**Registered Public Keys:**
| User | Key |
|---|---|
| alice | SIG_alice99 |
| bob | SIG_bob88 |

**Token Exchange Graph:**
| Pair | Risk |
|---|---|
| BTC ↔ ETH | 0.05 |
| ETH ↔ USDT | 0.01 |
| BTC ↔ USDT | 0.07 |

**Data Chunks:**
| Chunk ID | Size |
|---|---|
| C001 | 400 KB |
| C002 | 700 KB |

**Pre-run Transactions:**
- alice → bob: 100 ETH
- charlie → alice: 1.0 BTC

---

## Sample Input/Output

### Example 1: Check Balance (Option 2)
```
ID: alice

User ID: alice
Token: ETH
Balance: 900
```
*(Balance is 900 after the preloaded alice→bob transaction of 100)*

### Example 2: Verify Signature (Option 10)
```
User: bob
Key: SIG_bob88

Signature VALID.
```

### Example 3: Rollback Transaction (Option 5)
```
Rollback successful!
```
*(Reverses the most recent transaction on the stack)*

### Example 4: Safest Exchange Path (Option 15)
```
Start Token: ETH
End Token: USDT

Safest path risk: 0.01
Path: ETH -> USDT
```

### Example 5: Sort Pool by Speed (Option 12)
```
# After adding: TX001 (slow), TX002 (fast), TX003 (medium)

Pool sorted by speed.
1. TX: TX002  Speed: fast
2. TX: TX003  Speed: medium
3. TX: TX001  Speed: slow
```

### Example 6: Pack Blocks (Option 17)
```
# With preloaded chunks C001 (400 KB) and C002 (700 KB)

Block 1 size: 400 chunks: C001
Block 2 size: 700 chunks: C002
```
*(Both chunks fit in separate blocks; together they would exceed 1000 KB)*

---

## Startup Demo Output

When the program launches, it automatically runs a signature verification demo:

```
--- MintChain ---
--- Signature Verification Demo ---
Checking alice with correct key:
Signature VALID.
Checking alice with wrong key:
Signature INVALID.
Checking unregistered user 'charlie':
Error: No key registered for user.
------------------------------------
```

---

## Project Structure

```
dsa-project/
├── main.cpp      # Full source code (single file)
├── main          # Compiled executable (Mac/Linux)
└── README.md     # This file
```
