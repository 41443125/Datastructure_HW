# 41443125

姓名：姜宥康

作業一

## 解題說明

本題要求實作 Ackermann Function，分別完成遞迴（Recursive）與非遞迴（Non-recursive）版本。

Ackermann Function 定義如下：

$$
A(m,n)=
\begin{cases}
n+1, & m=0 \\
A(m-1,1), & m>0,\ n=0 \\
A(m-1,A(m,n-1)), & m>0,\ n>0
\end{cases}
$$

### 輸入限制

本實作為避免 Ackermann Function 成長過快，測試時設定：

```text
0 <= m <= 3
0 <= n <= 10
```

當 $m=3$ 時：

$$
A(3,n)=2^{n+3}-3
$$

因此即使輸入只增加一點，計算量也會快速成長。

## 解題策略

### Recursive 版本

直接依照 Ackermann Function 的數學定義撰寫遞迴：

1. 當 $m=0$ 時，回傳 $n+1$。
2. 當 $m>0$ 且 $n=0$ 時，計算 $A(m-1,1)$。
3. 其他情況計算 $A(m-1,A(m,n-1))$。

### Non-recursive 版本

非遞迴版本不能直接呼叫函式本身，因此使用 stack 模擬原本的函式呼叫堆疊。

由於本題限制只能使用 `iostream`、`algorithm`、`string`、`cmath`，不能使用 `vector` 或 `stack`，因此以 `string` 模擬 `vector<long long>`：

- 每 8 個 `char` 儲存一個 `long long`。
- `pushstr()`：將 `long long` 拆成 8 個 byte 後加入 `string` 尾端。
- `valstr()`：讀取任意索引的 8 個 byte，重新組合成 `long long`。
- `erasestr()`：刪除指定索引的 8 個 byte。

在 Ackermann 演算法中只會刪除最後一個元素，因此 `erasestr(st, top)` 的效果等價於 stack 的 `pop_back()`。

## 程式實作

### Recursive

```cpp
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
```

### Non-recursive

```cpp
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
```

## 效能分析

### 時間複雜度

Recursive 與 Non-recursive 版本展開的是相同的 Ackermann 計算流程，因此在固定 $m$ 的情況下，主要時間複雜度相同。

| $m$ | Ackermann 成長 | 時間複雜度 |
|---:|---|---:|
| 0 | $A(0,n)=n+1$ | $O(1)$ |
| 1 | $A(1,n)=n+2$ | $O(n)$ |
| 2 | $A(2,n)=2n+3$ | $O(n^2)$ |
| 3 | $A(3,n)=2^{n+3}-3$ | $O(4^n)$ |

在本實作限制 $0\le m\le3$ 下，最壞情況發生於 $m=3$，因此：

$$
\boxed{O(4^n)}
$$

若不限制 $m$，Ackermann Function 的成長速度會超過一般固定次數的指數函數，因此不能單純使用 $O(4^n)$ 描述所有 $m$ 的情況。

### Fake vector 各操作時間複雜度

令 $N=\text{st.size()}/8$，表示目前 fake vector 內的 `long long` 元素數量。

| 操作 | 時間複雜度 | 說明 |
|---|---:|---|
| `st.size()` | $O(1)$ | `string` 直接保存目前長度 |
| `st.empty()` | $O(1)$ | 只需判斷目前長度是否為 0 |
| `valstr(st, index)` | $O(1)$ | 固定讀取 8 個 byte |
| `pushstr(st, value)` | amortized $O(1)$ | 固定加入 8 個 byte；重新配置容量時單次最壞可到 $O(N)$ |
| `erasestr(st, index)` | $O(N)$ | 一般情況下，刪除中間元素需要搬移後方資料 |
| `erasestr(st, top)` | $O(1)$ | 本程式只刪最後 8 個 byte，等價於 `pop_back` |

因此在 Non-recursive Ackermann 的 `while` 迴圈中，每次 fake vector 的操作皆可視為 amortized $O(1)$；整體時間主要由 Ackermann 展開次數決定。

### 空間複雜度

#### Recursive

Recursive 版本需要使用系統 call stack 保存尚未完成的函式呼叫。Ackermann 的遞迴深度會隨結果快速增加，在 $m=3$ 時可視為：

$$
O(A(3,n))=O(2^n)
$$

因此在本實作限制下，Recursive 版本最壞空間複雜度為：

$$
\boxed{O(2^n)}
$$

#### Non-recursive

Non-recursive 版本改用 `string st` 保存待處理的 $m$。每一個元素固定使用 8 個 `char`，常數 8 不影響 Big-O。

最大 stack 大小與 Ackermann 計算過程中的待處理項目數量同階，在 $m=3$ 時為 $O(A(3,n))$，因此：

$$
\boxed{O(2^n)}
$$

雖然 Non-recursive 版本避免使用系統遞迴 call stack，但仍需要自行配置相同概念的 stack 空間。

## 測試與驗證

### 測試案例

| 測試案例 | 輸入 $(m,n)$ | 預期輸出 |
|---|---:|---:|
| 測試一 | $(0,5)$ | 6 |
| 測試二 | $(1,5)$ | 7 |
| 測試三 | $(2,3)$ | 9 |
| 測試四 | $(3,2)$ | 29 |
| 測試五 | $(3,4)$ | 125 |

Recursive 與 Non-recursive 版本對相同輸入應得到完全相同的結果。

### 編譯與執行指令

```shell
g++ -std=c++17 -o ackermann ackermann.cpp
./ackermann
```

例如輸入：

```text
2 3
```

輸出：

```text
9
```


## 申論

Ackermann Function 的數學定義本身具有高度遞迴性，因此 Recursive 版本可以直接依照公式實作，程式碼短且容易驗證正確性。當遇到 $A(m-1,A(m,n-1))$ 時，系統會自動利用 call stack 保存尚未完成的外層運算，因此程式設計上最直觀。然而，這種寫法的缺點是遞迴深度會隨 Ackermann Function 的成長快速增加，當輸入稍大時，除了運算時間大幅上升之外，也可能因 call stack 過深而發生 Stack Overflow。

Non-recursive 版本的核心則是將原本由系統 call stack 負責的工作改成由程式自行管理。當計算

$$
A(m,n)=A(m-1,A(m,n-1))
$$

時，必須先保存外層尚未處理的 $m-1$，再優先計算內層 $A(m,n-1)$。因此本程式使用 LIFO 的方式保存待處理的 $m$，其行為與 stack 相同。這也說明了遞迴與非遞迴版本在演算法本質上並沒有改變，只是將「隱藏在系統中的堆疊」改成「由程式明確維護的堆疊」。

由於題目限制只能使用那幾個標頭，無法直接使用 `vector` 或 `stack`，因此本實作利用 `string` 模擬 `vector<long long>`。每個 `long long` 固定拆成 8 個 byte 儲存，`pushstr()` 負責加入資料，`valstr()` 負責依索引讀取資料，`erasestr()` 負責刪除資料。這種設計的優點是可以在不引入其他容器函式庫的情況下，仍然建立出具有 push、隨機存取與 erase 功能的簡易容器，也能實際理解資料在記憶體中的 byte 表示方式。

不過，這種 fake vector 也有代價。一般情況下 `string::erase()` 若刪除中間元素，需要搬移後面的資料，因此時間複雜度為 $O(N)$；本題只刪除最後一個元素，所以可視為 $O(1)$。此外，`pushstr()` 雖然平常只加入固定 8 個 byte，但當 `string` 容量不足而重新配置記憶體時，單次操作仍可能需要複製既有內容，因此更精確地說是 amortized $O(1)$。

綜合來看，Recursive 版本的優點是簡潔、接近數學定義；Non-recursive 版本則能展示如何手動模擬函式呼叫堆疊，也避免直接使用系統遞迴。兩者的時間複雜度並不會因為改成迭代就大幅降低，因為真正造成大量計算的是 Ackermann Function 本身極快的成長速度。Non-recursive 的主要價值在於控制堆疊的方式與實作方法不同，而不是把 Ackermann Function 變成低複雜度的問題。

## 結論

1. Recursive 版本直接依照 Ackermann Function 的數學定義實作，程式簡潔且容易理解。
2. Non-recursive 版本以 `string` 模擬 `vector<long long>` 與 stack，不需要使用 `vector` 或 `stack` 函式庫。
3. 每一個 `long long` 使用固定 8 個 `char` 編碼，透過 `pushstr()`、`valstr()` 與 `erasestr()` 完成 push、隨機讀取與刪除操作。
4. 在 $0\le m\le3$ 的限制下，最壞時間複雜度為 $O(4^n)$，最壞額外空間複雜度為 $O(2^n)$。
5. Ackermann Function 成長非常快速，因此即使輸入值不大，也可能產生大量運算與堆疊使用量。
