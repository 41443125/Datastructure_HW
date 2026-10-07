#include <iostream>
#include <algorithm>
#include <string>
#include <cmath>
using namespace std;
using ll = long long;
using llu = unsigned long long;
void pushstr(string& st, ll value) {
    llu x = (llu)value;
    st.push_back((unsigned char)(x & 0xffULL));
    st.push_back((unsigned char)((x & 0xff00ULL) >> 8));
    st.push_back((unsigned char)((x & 0xff0000ULL) >> 16));
    st.push_back((unsigned char)((x & 0xff000000ULL) >> 24));
    st.push_back((unsigned char)((x & 0xff00000000ULL) >> 32));
    st.push_back((unsigned char)((x & 0xff0000000000ULL) >> 40));
    st.push_back((unsigned char)((x & 0xff000000000000ULL) >> 48));
    st.push_back((unsigned char)((x & 0xff00000000000000ULL) >> 56));
}
ll valstr(const string& st, int index) {
    int p = index * 8;
    llu x = 0;
    x |= (llu)(unsigned char)st[p];
    x |= (llu)(unsigned char)st[p + 1] << 8;
    x |= (llu)(unsigned char)st[p + 2] << 16;
    x |= (llu)(unsigned char)st[p + 3] << 24;
    x |= (llu)(unsigned char)st[p + 4] << 32;
    x |= (llu)(unsigned char)st[p + 5] << 40;
    x |= (llu)(unsigned char)st[p + 6] << 48;
    x |= (llu)(unsigned char)st[p + 7] << 56;

    return (ll)x;
}
void erasestr(string& st, int index) {
    st.erase(index * 8, 8);
}
ll Ackermann(ll m, ll n) {
    string st="";// fake vector
    pushstr(st, m);
    while (!st.empty()) {
        int top = st.size() / 8 - 1;
        m = valstr(st, top);
        erasestr(st, top);
        if (m == 0) {// A(0, n) = n + 1
            n++;
        }
        else if (n == 0) {// A(m, 0) = A(m - 1, 1)
            n = 1;
            pushstr(st, m - 1);
        }
        else {// A(m, n) = A(m - 1, A(m, n - 1))
            n--;
            pushstr(st, m - 1);
            pushstr(st, m);
        }
    }
    return n;
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
//st.size()                 O(1)
//valstr(st, top)           O(1)
//erasestr(st, top)         O(1)只訪問最後一個元素時等價於pop_back
//pushstr(st, val)          O(1)
//st.empty()                O(1)