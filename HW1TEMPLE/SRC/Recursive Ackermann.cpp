#include <iostream>
#include <algorithm>
#include <string>
#include <cmath>
using namespace std;
using ll = long long;
ll Ackermann(ll m, ll n) {
    if (m == 0) {
        return n + 1;
    }
    else if (n == 0) {
        return Ackermann(m - 1, 1);
    }
    else {
        return Ackermann(m - 1, Ackermann(m, n - 1));
    }
}
int main() {
        ll m, n;
        cin >> m >> n;
        cout << Ackermann(m, n) << endl;
}
//limit
//0 <= m <= 3
//0 <= n <= 10
//Big O
//m	 	時間複雜度
//0		O(1)
//1		O(n)
//2		O(n²)
//3		O(4ⁿ)