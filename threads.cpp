/*
CPSC 351 - Multithreaded Bank Simulation

Group Members:
- Britaney Do (Step 1)
- Bassma Ennarah (Step 2)
- Elizabeth Philip
- Sehaj Dhaliwal
*/


#include <iostream>
#include <vector>
#include <thread>
#include <random>

using namespace std;

class BankAccount
{
public:
    int balance;

    BankAccount(int start)
    {
        balance = start;
    }

    void deposit(int amount)
    {
        int current = balance;
        balance = current + amount;
    }

    void withdraw(int amount)
    {
        int current = balance;
        balance = current - amount;
    }

    int returnBalance()
    {
        return balance;
    }
};

void simulatedUser(BankAccount& account, int transactions, int seed)
{
    mt19937 rng(seed);
    uniform_int_distribution<int> actionDist(0, 1);   // 0 deposit, 1 withdraw
    uniform_int_distribution<int> amountDist(1, 50);  // 1..50

    for (int i = 0; i < transactions; i++)
    {
        int amount = amountDist(rng);
        int action = actionDist(rng);

        if (action == 0) account.deposit(amount);
        else account.withdraw(amount);
    }
}

int main()
{
    BankAccount account(1000);

    const int numThreads = 4;
    const int transactionsPerThread = 100;

    vector<thread> threads;
    threads.reserve(numThreads);

    random_device rd;

    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back(simulatedUser, ref(account), transactionsPerThread, rd());
    }

    for (auto& t : threads) t.join();

    cout << "Final balance: " << account.returnBalance() << endl;
    return 0;
}