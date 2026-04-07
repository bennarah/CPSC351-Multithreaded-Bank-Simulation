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
#include <mutex>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <functional>

using namespace std;

// a class which can be accessed by the threads
class BankAccount
{
public:
    int balance;

    // added mutex variable
    mutex mutexHelp;

    BankAccount(int start)
    {
        balance = start;
    }

    void deposit(int amount)
    {
        // added Step 5a that is given to artifically delay to increase multiple threads
        this_thread::sleep_for(chrono::microseconds(5));

        // added mutex lock and unlock. now threads should not have race conditions
        mutexHelp.lock();
        int current = balance;
        balance = current + amount;
        mutexHelp.unlock();

    }

    void withdraw(int amount)
    {
        // Step 5 again
        this_thread::sleep_for(chrono::microseconds(5));

        // added mutex lock and unlock to remove race conditions
        mutexHelp.lock();
        int current = balance;
        balance = current - amount;
        mutexHelp.unlock();
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
void childWork(BankAccount* shared_account, int writeFd, int childId)
{
    mt19937 rng(time(nullptr) + childId);
    uniform_int_distribution<int> actionDist(0, 1);
    uniform_int_distribution<int> amountDist(1, 50);

    ostringstream log;
    log << "\n--- Child " << childId << " Transaction Log ---\n";

    for (int i = 0; i < 20; i++)
    {
        int amount = amountDist(rng);
        int action = actionDist(rng);

        if (action == 0)
        {
            shared_account->deposit(amount);
            log << "Child " << childId << ": Deposit +" << amount << endl;
        }
        else
        {
            shared_account->withdraw(amount);
            log << "Child " << childId << ": Withdraw -" << amount << endl;
        }
    }

    string output = log.str();
    write(writeFd, output.c_str(), output.size());
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

        BankAccount account(startBalance);
        vector<thread> threads;
        vector<int> netChanges(numThreads, 0);
        threads.reserve(numThreads);

        for (int i = 0; i < numThreads; i++)
        {
            threads.emplace_back(simulatedUser,
                     std::ref(account),
                     transactionsPerThread,
                     static_cast<int>(rd()),
                     i + 1,
                     run,
                     std::ref(netChanges[i]));
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

    // mmap implementation (shared memory)
    // create shared memory for a BankAccount
    // copied from class example
    BankAccount* shared_account = (BankAccount*) mmap(
        NULL,                        // let system choose address
        sizeof(BankAccount),         // size = 1 BankAccount
        PROT_READ | PROT_WRITE,      // read/write
        MAP_SHARED | MAP_ANONYMOUS,
        -1, 0
    );
 
    // initialize the shared account w starting balance
    shared_account->balance = startBalance;
    cout << "Initial balance in parent: " << shared_account->returnBalance() << endl;
 
    int fd[2];
    if (pipe(fd) == -1)
    {
        cerr << "Pipe creation failed" << endl;
        return 1;
    }
    // fd[0] is the read end of the pipe
    // fd[1] is the write end of the pipe
    cout << "Pipe created successfully." << endl;
    pid_t pid1 = fork();

if (pid1 == 0)
{
    close(fd[0]); // child 1 doesn't read
    childWork(shared_account, fd[1], 1);
    close(fd[1]);
    exit(0);
}

pid_t pid2 = fork();

if (pid2 == 0)
{
    close(fd[0]); // child 2 doesn't read
    childWork(shared_account, fd[1], 2);
    close(fd[1]);
    exit(0);
}

// parent
close(fd[1]);

char buffer[4096];
ssize_t bytesRead;

cout << "\n--- Merged Logs From Children ---" << endl;

while ((bytesRead = read(fd[0], buffer, sizeof(buffer) - 1)) > 0)
{
    buffer[bytesRead] = '\0';
    cout << buffer;
}

close(fd[0]);

waitpid(pid1, NULL, 0);
waitpid(pid2, NULL, 0);

cout << "\nFinal shared account balance: "
     << shared_account->returnBalance() << endl;

    return 0;
}
