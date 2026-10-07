#include <iostream>
#include <algorithm>
#include <string>
#include <cmath>


using namespace std;
using ll = long long;
using llu = unsigned long long;
void subset(string S[], string temp[],int n, int start, int k, int count) {
        if (count == k)
        {
                cout << "(";
                for (int i = 0; i < count; i++)
                {
                        cout << temp[i];
                        if (i != count - 1)
                                cout << ",";
                }
                cout << ")";
                return;
        }
        for (int i = start; i < n; i++)
        {
                temp[count] = S[i];
                subset(S, temp,n,i + 1,k,count + 1);
        }
}
void powerset(string S[], int n)
{
        string* temp = new string[n];
        cout << "{";
        for (int k = 0; k <= n; k++)
        {
                if (k != 0)
                        cout << ", ";
                subset(S, temp, n, 0, k, 0);
        }
        cout << "}";
        delete[] temp;
}
int main() {
        int n;
        cin >> n;
        string* S = new string[n];
        for (int i = 0; i < n; i++)
        {
                cin >> S[i];
        }
        powerset(S, n);
        delete[] S;
}
//limit
//0 <= n <= 20
//Big O
//powerset(S, n)                         O(n * 2^n)
//subset(S, temp, n, start, k, count)    O(n * 2^n)  // 所有呼叫合計
//temp[count] = S[i]                     O(1)        // 假設每個元素長度固定
//cout << temp[i]                        O(1)        // 假設每個元素長度固定
//new string[n]                          O(n)
//delete[] temp                          O(n)
//空間
//temp                                   O(n)
//recursive call stack                   O(n)
//total auxiliary space                  O(n)