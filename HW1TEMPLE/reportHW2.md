# 41443125

姓名：姜宥康

作業二

## 解題說明

本題要求撰寫一個遞迴函式，計算集合 \(S\) 的 powerset（冪集合）。

若集合 \(S\) 有 \(n\) 個元素，則 powerset 為 \(S\) 所有可能子集合所形成的集合。

例如：

\[
S=(a,b,c)
\]

則：

\[
powerset(S)=\{(),(a),(b),(c),(a,b),(a,c),(b,c),(a,b,c)\}
\]

每一個元素都有「選」與「不選」兩種可能，因此 powerset 的子集合總數為：

\[
|P(S)|=2^n
\]

本實作為了讓輸出順序與題目範例一致，會依照子集合大小依序產生：

1. 先產生大小為 0 的子集合。
2. 再產生大小為 1 的子集合。
3. 接著產生大小為 2 的子集合。
4. 持續到大小為 \(n\) 的子集合。

### 輸入限制

題目本身未給定明確的 \(n\) 上限。由於 powerset 共有 \(2^n\) 個子集合，輸出量會隨 \(n\) 指數成長，因此本報告測試時設定：

```text
0 <= n <= 20
```

當 \(n=20\) 時，子集合數量已經為：

\[
2^{20}=1,048,576
\]

因此即使 \(n\) 只增加一點，輸出資料量也會快速增加。

## 解題策略

本題使用遞迴產生所有子集合。

為了讓輸出順序與題目範例一致，`powerset()` 會依序要求產生大小為 \(0,1,2,\dots,n\) 的子集合，再利用 `subset()` 遞迴完成固定大小的組合。

### `powerset()` 的工作

`powerset()` 依序設定目標子集合大小：

```text
0
1
2
...
n
```

每一個大小都呼叫一次 `subset()`。

例如 \(S=(a,b,c)\)：

```text
size = 0   -> ()
size = 1   -> (a), (b), (c)
size = 2   -> (a,b), (a,c), (b,c)
size = 3   -> (a,b,c)
```

### `subset()` 的工作

`subset()` 使用以下資訊進行遞迴：

- `S[]`：原始集合。
- `temp[]`：目前已選到的元素。
- `start`：下一次可以開始選取的位置。
- `targetSize`：目前要求的子集合大小。
- `count`：目前已經選取的元素數量。
- `first`：控制輸出時是否需要先印逗號。

當：

```cpp
count == targetSize
```

表示已經找到一個符合大小的子集合，此時直接輸出。

否則從 `start` 開始選下一個元素，將元素放入 `temp[count]`，再遞迴處理下一層。

例如產生大小為 2 的子集合時：

```text
a
├── b  -> (a,b)
└── c  -> (a,c)

b
└── c  -> (b,c)
```

由於下一層只從目前元素的下一個位置開始，因此不會產生 `(b,a)` 這類重複排列。

## 程式實作

### Recursive Powerset

```cpp
#include <iostream>
#include <algorithm>
#include <string>
#include <cmath>

using namespace std;

void subset(string S[],
            string temp[],
            int n,
            int start,
            int targetSize,
            int count,
            bool& first)
{
    if (count == targetSize)
    {
        if (!first)
            cout << ", ";

        first = false;

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

    // 剩餘元素數量必須足夠填滿 targetSize
    for (int i = start;
         i <= n - (targetSize - count);
         i++)
    {
        temp[count] = S[i];

        subset(
            S,
            temp,
            n,
            i + 1,
            targetSize,
            count + 1,
            first
        );
    }
}

void powerset(string S[], int n)
{
    string* temp = new string[n];

    bool first = true;

    cout << "{";

    for (int size = 0; size <= n; size++)
    {
        subset(
            S,
            temp,
            n,
            0,
            size,
            0,
            first
        );
    }

    cout << "}";

    delete[] temp;
}

int main()
{
    int n;
    cin >> n;

    string* S = new string[n];

    for (int i = 0; i < n; i++)
    {
        cin >> S[i];
    }

    powerset(S, n);

    cout << endl;

    delete[] S;

    return 0;
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

## 效能分析

### 時間複雜度

若集合中有 \(n\) 個元素，大小為 \(k\) 的子集合共有：

\[
\binom{n}{k}
\]

因此 powerset 的總子集合數量為：

\[
\sum_{k=0}^{n}\binom{n}{k}=2^n
\]

如果只看「有多少個子集合」，至少需要處理 \(2^n\) 個結果。

但本程式還必須實際輸出每個子集合中的元素。

所有子集合中出現的元素總數為：

\[
\sum_{k=0}^{n}k\binom{n}{k}
=
n2^{n-1}
\]

因此包含實際輸出成本後，整體時間複雜度為：

\[
\boxed{O(n2^n)}
\]

這也是 powerset 類型問題無法避免的成本，因為輸出本身就具有指數數量。

### 主要操作時間複雜度

假設集合中的每一個元素都是固定長度字串，則：

| 操作 | 時間複雜度 | 說明 |
|---|---:|---|
| `temp[count] = S[i]` | \(O(1)\) | 固定長度元素複製 |
| `count == targetSize` | \(O(1)\) | 單純整數比較 |
| `first` 判斷 | \(O(1)\) | 布林值判斷 |
| 單次陣列索引 | \(O(1)\) | 直接存取指定位置 |
| 輸出一個大小為 \(k\) 的 subset | \(O(k)\) | 需要輸出 \(k\) 個元素 |
| 輸出全部 powerset | \(O(n2^n)\) | 全部子集合元素總量為 \(n2^{n-1}\) |

若集合元素本身可能是很長的字串，則字串複製與輸出的成本還需要再乘上字串長度。

### 空間複雜度

程式使用：

```cpp
string* temp = new string[n];
```

保存目前正在建立的子集合，因此需要：

\[
O(n)
\]

額外空間。

此外，遞迴最深時最多選取 \(n\) 個元素，因此 recursive call stack 的最大深度為：

\[
O(n)
\]

所以整體額外空間複雜度為：

\[
\boxed{O(n)}
\]

若把輸入集合 `S` 本身也計算進去，仍然是 \(O(n)\)。

## 測試與驗證

### 測試案例

| 測試案例 | 輸入 | 預期輸出 |
|---|---|---|
| 測試一 | `0` | `{()}` |
| 測試二 | `1`、`a` | `{(), (a)}` |
| 測試三 | `2`、`a b` | `{(), (a), (b), (a,b)}` |
| 測試四 | `3`、`a b c` | `{(), (a), (b), (c), (a,b), (a,c), (b,c), (a,b,c)}` |

### 編譯與執行指令

```shell
g++ -std=c++17 -o powerset powerset.cpp
./powerset
```

例如輸入：

```text
3
a b c
```

輸出：

```text
{(), (a), (b), (c), (a,b), (a,c), (b,c), (a,b,c)}
```

與題目範例的 powerset 內容相同。

## 申論

Powerset 的核心概念是：對集合中的每一個元素，都存在「加入子集合」與「不加入子集合」的可能，因此 \(n\) 個元素會形成 \(2^n\) 個不同的子集合。這也是為什麼 powerset 的輸出數量會呈指數成長。

本實作沒有使用 `vector`、`stack` 或其他額外容器，而是使用動態陣列 `string* temp` 保存目前遞迴選取的元素。`temp[count]` 表示目前遞迴深度所選取的元素，當遞迴返回上一層時，下一個元素可以直接覆蓋同一個位置，因此不需要真正執行 `erase` 或 `pop_back`。

如果單純使用每個元素「選或不選」的二元遞迴，可以很直接地產生全部 \(2^n\) 個子集合。不過這種方法的輸出順序通常會是依照遞迴樹走訪順序，而不一定與題目範例中「先列出空集合，再列出單元素集合，再列出雙元素集合」的順序相同。

因此本程式改成先指定 `targetSize`，再遞迴產生固定大小的所有組合。當 `targetSize=1` 時產生所有單元素集合，`targetSize=2` 時產生所有雙元素集合，依此類推。這種方式可以直接得到與題目範例相同的排列方式。

在遞迴過程中，`start` 用來限制下一個元素只能從目前位置之後選取。例如已經選擇 `a` 之後，只能再從 `b`、`c` 中選，因此不會產生 `(b,a)` 或重複的 `(a,b)`。這使得每一個組合只會被產生一次。

時間複雜度方面，雖然 powerset 只有 \(2^n\) 個子集合，但實際輸出時，每個子集合最多需要輸出 \(n\) 個元素。全部子集合中總共會輸出 \(n2^{n-1}\) 個元素，因此完整計算與輸出的時間複雜度為 \(O(n2^n)\)。這個成本主要來自 powerset 本身的輸出規模，而不是單純因為遞迴寫法效率差。

空間方面，只需要一個長度為 \(n\) 的暫存陣列，以及最深 \(n\) 層的遞迴 call stack，因此額外空間複雜度為 \(O(n)\)。相較於先把全部 \(2^n\) 個子集合儲存起來，本方法會在找到子集合後直接輸出，因此不需要 \(O(n2^n)\) 的額外儲存空間。

## 結論

1. Powerset 共有 \(2^n\) 個子集合，因此問題本身具有指數級的輸出量。
2. 本實作使用遞迴產生所有子集合，符合題目要求。
3. 為了讓輸出順序與題目範例一致，程式依照子集合大小 \(0,1,2,\dots,n\) 依序產生組合。
4. 程式只使用 `iostream`、`algorithm`、`string`、`cmath`，沒有使用 `vector`、`stack`、`struct` 或自訂 `class`。
5. 完整輸出所有 powerset 的時間複雜度為 \(O(n2^n)\)。
6. 暫存陣列與遞迴 call stack 的額外空間複雜度皆為 \(O(n)\)，因此總額外空間為 \(O(n)\)。
