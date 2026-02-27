/*
CPSC 351 - Multithreaded Bank Simulation

Group Members:
- Britaney Do (Step 1)
- Bassma Ennarah (Step 2)
- Elizabeth Philip (Step 3)
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

void simulatedUser(BankAccount& account,
                   int transactions,
                   int seed,
                   int& netChange)
{
    mt19937 rng(seed);
    uniform_int_distribution<int> actionDist(0, 1);
    uniform_int_distribution<int> amountDist(1, 50);

    netChange = 0;

    for (int i = 0; i < transactions; i++)
    {
        int amount = amountDist(rng);
        int action = actionDist(rng);

        if (action == 0) {
            account.deposit(amount);
            netChange += amount;
        }
        else {
            account.withdraw(amount);
            netChange -= amount;
        }
    }
}

int main()
{
    const int startBalance = 1000;
    const int numThreads = 4;
    const int transactionsPerThread = 100;
    const int numRuns = 100;

    int raceCount = 0;
    random_device rd;

    for (int run = 1; run <= numRuns; run++)
    {
        cout << "\n========== Run " << run << " ==========" << endl;

        // Fresh account each run
        BankAccount account(startBalance);

        vector<thread> threads;
        vector<int> netChanges(numThreads, 0);

        threads.reserve(numThreads);

        // Create threads
        for (int i = 0; i < numThreads; i++)
        {
            threads.emplace_back(
                simulatedUser,
                ref(account),
                transactionsPerThread,
                rd(),                    // random seed
                ref(netChanges[i])       // track net change per thread
            );
        }

        // Wait for all threads
        for (auto& t : threads)
        {
            t.join();
        }

        // Calculate expected balance (sequential result)
        int expectedBalance = startBalance;
        for (int i = 0; i < numThreads; i++)
        {
            expectedBalance += netChanges[i];
        }

        int finalBalance = account.returnBalance();

        cout << "Expected Balance: " << expectedBalance << endl;
        cout << "Final Balance:   " << finalBalance << endl;

        if (expectedBalance != finalBalance)
        {
            cout << ">>> Race condition detected!" << endl;
            raceCount++;
        }
        else
        {
            cout << "No race condition this run." << endl;
        }
    }

    cout << "\n======================================" << endl;
    cout << "Total runs with race conditions: "
         << raceCount << " / " << numRuns << endl;

    return 0;
}
