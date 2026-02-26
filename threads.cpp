/*
CPSC 351 - Multithreaded Bank Simulation

Step 1 - BankAccount class: Britaney

Group Members:
- Britaney Do
- Bassma Ennarah
- Elizabeth Philip
- Sehaj Dhaliwal
*/

#include <iostream>
#include <vector>
#include <thread>
using namespace std;

class BankAccount
{
    public:

    int balance;

    BankAccount(int start)
    {
        balance = start;
    }

    // deposit into BankAccount
    // implementing both read AND write creates race conditions
    void deposit(int amount)
    {   
        // read current balance
        int current = balance;

        // write new balance
        balance = current + amount;
    }

    // withdraw from BankAccount
    void withdraw(int amount)
    {
        // read current balance
        int current = balance;

        // write new balance
        balance = current - amount;
    }

    // return balance in BankAccount
    int returnBalance()
    {
        return balance;
    }
};