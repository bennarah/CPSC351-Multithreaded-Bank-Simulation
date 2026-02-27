/*
CPSC 351 - Multithreaded Bank Simulation

Group Members:
- Britaney Do (Step 1)
- Bassma Ennarah (Step 2)
- Elizabeth Philip (Step 3 & 5a)
- Sehaj Dhaliwal (Step 4)
*/

#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <sstream>
#include <fstream>
#include <chrono>

using namespace std;

// a class which can be accessed by the threads
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
        // added Step 5a that is given to artifically delay to increase multiple threads
        this_thread::sleep_for(chrono::microseconds(5));

        int current = balance;
        balance = current + amount;
    }

    void withdraw(int amount)
    {
        // Step 5 again
        this_thread::sleep_for(chrono::microseconds(5));

        int current = balance;
        balance = current - amount;
    }

    int returnBalance()
    {
        return balance;
    }
};

//  Thread Function
void simulatedUser(BankAccount& account,
                   int transactions,
                   int seed,
                   int threadId,
                   int runId,
                   int& netChange)
{
    mt19937 rng(seed);//random number generator 
    uniform_int_distribution<int> actionDist(0, 1);
    uniform_int_distribution<int> amountDist(1, 50);

    netChange = 0;
    ostringstream log;

    for (int i = 0; i < transactions; i++)
    {
        int amount = amountDist(rng);
        int action = actionDist(rng);

        if (action == 0)
        {
            account.deposit(amount);
            netChange += amount;
            log << "Deposit +" << amount << endl;
        }
        else
        {
            account.withdraw(amount);
            netChange -= amount;
            log << "Withdraw -" << amount << endl;
        }
    }
//step 4
    string filename = "log_run_" + to_string(runId) + "_thread_" + to_string(threadId) + ".txt";
    ofstream out(filename);
    out << log.str();
    out.close();
}

// Step 3 + 4: Main Simulation

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

        BankAccount account(startBalance);
        vector<thread> threads;
        vector<int> netChanges(numThreads, 0);
        threads.reserve(numThreads);

        for (int i = 0; i < numThreads; i++)
        {
            threads.emplace_back(simulatedUser,
                                 ref(account),
                                 transactionsPerThread,
                                 rd(),
                                 i + 1,
                                 run,
                                 ref(netChanges[i]));
        }


        for (auto& t : threads)
            t.join();//wait for threads to finish

        // Compute expected balance from net changes
        int expectedBalance = startBalance;
        for (int change : netChanges)
            expectedBalance += change;

        int finalBalance = account.returnBalance();

        cout << "Expected Balance: " << expectedBalance << endl;
        cout << "Final Balance:    " << finalBalance << endl;

        if (expectedBalance != finalBalance)
        {
            cout << ">>> Race condition detected!" << endl;
            raceCount++;
        }
        else
        {
            cout << "No race condition this run." << endl;
        }

        //Merge thread logs
        cout << "\n--- Merged Transaction Log (Run " << run << ") ---\n";
        ostringstream mergedLog;
        for (int i = 0; i < numThreads; i++)
        {
            string filename = "log_run_" + to_string(run) + "_thread_" + to_string(i + 1) + ".txt";
            ifstream in(filename);
            string line;
            while (getline(in, line))
            {
                mergedLog << "Thread " << i + 1 << ": " << line << endl;
            }
            in.close();
        }
        cout << mergedLog.str();
    }
//runs are complete
    cout << "\n======================================" << endl;
    cout << "Total runs with race conditions: "
         << raceCount << " / " << numRuns << endl;

    return 0;
}
