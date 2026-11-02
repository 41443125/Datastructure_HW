# 41443125

# 姓名：姜宥康

## 解題說明

本題實作多項式類別，提供零多項式建構、加法、乘法及代入計算，並多載 `>>`、`<<` 運算子完成輸入與輸出。

若多項式表示為：

\[
A(x)=\sum_{i=0}^{m-1}a_i x^{e_i}
\]

其中 \(a_i\) 為係數，\(e_i\) 為非負整數指數，則加法需要合併同次項，乘法則需要將各項配對相乘，再整理相同指數的結果。

例如：

\[
A(x)=3x^2+2x+1,\qquad B(x)=x-1
\]

則：

\[
A(x)+B(x)=3x^2+3x
\]

\[
A(x)B(x)=3x^3-x^2-x-1
\]

代入 \(x=2\) 時：

\[
A(2)=3\times2^2+2\times2+1=17
\]

本實作以 `u32string` 保存大數，係數與代入值先乘 100，轉成大整數處理。輸入超過兩位的小數直接捨去，運算結果則四捨五入到兩位，輸出時省略小數尾端的 0。

### 輸入限制

每筆多項式必須在同一行輸入完成，格式為：

```text
項數 係數0 指數0 係數1 指數1 ...
```

| 欄位 | 規定 |
|---|---|
| 項數 | 非負整數寫法，不可包含小數點；`0` 表示零多項式，項數需要是大於等於`0`的整數 |
| 係數 | 接受正負整數或小數；超過兩位小數直接捨去 |
| 指數 | 不接受負數；非負小數捨去小數部分後作為整數指數 |
| 代入值 x | 一行只匹配一個數字，超過兩位小數直接捨去 |

例如 `2.345` 保存為 `2.34`，`-2.345` 保存為 `-2.34`；指數 `20.1` 則保存為 `20`。指數會在截斷前檢查負數，因此 `-0.001` 不合法，`-0.00` 則視為零。

程式依序匹配數字，英文字母、`^` 等其他字元作為分隔。以下輸入都表示 `3x^4 + x^2`：

```text
2 1 2 3.0 4.0
2 1^2 3.0^4.0
a2a1.000000a02a3.00a4.0001a
```

正負號只保留最靠近數字的那個，例如 `-+1` 視為 `+1`、`+-1` 視為 `-1`，不對連續符號做代數化簡。程式也不將科學記號 `1e3` 解讀成 1000。

匹配到的係數、指數必須剛好有指定的項數。缺少資料、多出資料、沒有數字或數字格式錯誤時，整行丟棄並輸出 `輸入錯誤請重新輸入`，不從下一行補齊資料。重新輸入 A、B 失敗時，原物件保留。

大數版沒有額外設定項數或數字位數上限，實際範圍受記憶體、執行時間、堆疊，以及環境中的索引與尺度型別限制。

### 整行匹配與資料檢查

輸入時先使用 `getline` 讀取一整行，再由 `NumberScanner` 依序取出數字。

第一個數字是項數，之後每兩個數字為一組係數、指數。程式不根據宣告項數先配置固定大小陣列，而是掃描實際出現的資料，最後比較組數是否一致。

每次讀入多項式都先建立暫存物件，確認解析、排序及合併成功後，才替換原物件，避免輸入失敗時破壞已保存的 A、B。

### 大數與兩位小數的保存方式

`BigInteger` 使用小端序的 100 進位群組，每個 `u32string` 元素保存 0～99，正負號另外保存。

例如：

```text
原始數值：123456.78
乘以 100：12345678
低位到高位的群組：[78, 56, 34, 12]
```

`Fixed2` 保存數值乘 100 後的大整數，例如 `0.01 → 1`、`3 → 300`、`-1.25 → -125`。輸入直接從字元建立群組，不先將完整數值轉成 `int`、`long long` 或 `float`。

`DecimalPolynomial` 使用兩個 `u32string`：`data` 保存各項的指數、係數及其長度，`offsets` 保存每項的起點。長度與起點以兩個 32 位元儲存格保存，供目前的 64 位元索引使用。

指數本身也以大整數保存，因此不需要依最高次方配置密集陣列。

### 排序與多項式加法

輸入可能亂序或含有同次項，程式先用穩定合併排序將索引按指數降序排列，再依序合併相同指數的係數，並移除零項。

加法使用雙索引掃描：

1. 指數較大的項目先加入結果。
2. 指數相同時將係數相加，指數保留一份。
3. 一方掃描完成後，加入另一方剩餘項目。

例如 `3x^2 + 2x + 1` 加上 `x - 1`，一次項合併為 `3x`，常數項抵消，所以結果為 `3x^2 + 3x`。

### 大整數乘法

大整數乘法採卡拉楚巴演算法。令 \(B=100\)，在第 \(h\) 個群組處將兩個數分成高、低兩半：

\[
a=a_H B^h+a_L,\qquad b=b_H B^h+b_L
\]

分別計算：

\[
z_0=a_Lb_L
\]

\[
z_2=a_Hb_H
\]

\[
z_1=(a_L+a_H)(b_L+b_H)-z_0-z_2
\]

最後合併為：

\[
ab=z_2B^{2h}+z_1B^h+z_0
\]

這樣可將四次半長度乘法減為三次。若任一運算元不超過 32 個 100 進位群組，就使用一般逐位乘法作為遞迴基底。

### 四捨五入與代入計算

係數各乘 100 後，相乘結果的單位為 1/10000。程式先將相同指數的精確乘積相加，再除以 100、四捨五入回兩位小數。

例如：

\[
(0.01x+0.01)(0.50x+0.50)
\]

一次項的兩個乘積都是 \(0.005x\)，先合併後得到 \(0.01x\)，因此結果為：

\[
0.01x^2+0.01x+0.01
\]

捨入以絕對值判斷，中點向遠離 0 的方向進位，所以 `0.005 → 0.01`、`-0.005 → -0.01`。

代入時使用快速冪，反覆檢查指數奇偶、將指數除以 2、將底數平方。中間值由 `ExactDecimal` 保存，其數值形式為：

\[
\frac{\text{units}}{100^{\text{scaleGroups}}}
\]

所有項目精確相加後才捨入，不在每次平方或每一項之後先截成兩位。x 為 0、1、-1 時有直接處理；指數為 0 時回傳 1，常數項在 x=0 時也能正確計算。

### 題目介面與主程式

程式保留題目指定的 `Term`、`Polynomial`、`Add`、`Mult`、`float Eval(float)` 及串流運算子。`Polynomial` 使用 `Term* termArray`、`capacity`、`terms`，配置自己的項目陣列，複製時逐項複製，解構時釋放。

題目相容類別運算時轉成大數核心，再驗證是否能保存回原生型別：指數受 `int` 範圍限制，浮點轉換則檢查兩位小數往返是否一致。主程式直接使用 `DecimalPolynomial`，提供完整的大數功能。

`main()` 每次讀取指令前顯示選單，`1`～`7` 分別對應顯示、加法、乘法、A 代入、B 代入、重新輸入 A、重新輸入 B。其他任何指令輸入直接結束；多項式及 x 格式錯誤則仍在同一欄位重試。

## 程式實作

### Polynomial

```cpp
#pragma GCC optimize("O2")
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

using namespace std;
namespace detail {
using Size = u32string::size_type;
void appendSize(u32string& packed, Size value) {
    packed.push_back(static_cast<char32_t>(value));
    packed.push_back(static_cast<char32_t>(value >> 32));
}

Size readSize(u32string& packed, Size position) {
    return static_cast<Size>(packed.at(position)) |
        (static_cast<Size>(packed.at(position + 1)) << 32);
}

void writeSize(u32string& packed, Size position, Size value) {
    packed.at(position) = static_cast<char32_t>(value);
    packed.at(position + 1) = static_cast<char32_t>(value >> 32);
}
bool isDigit(char character) {
    return character >= '0' && character <= '9';
}
bool beginsNumber(string inputLine, string::size_type position) {
    return position < inputLine.size() &&
        (isDigit(inputLine[position]) ||
         (inputLine[position] == '.' && position + 1 < inputLine.size() &&
          isDigit(inputLine[position + 1])));
}
class NumberScanner {// 只匹配十進位數字；其他字元是分隔符，緊鄰數字的正負號保留。
private:
    string inputLine;
    string::size_type position = 0;
public:
    NumberScanner(string sourceLine) : inputLine(sourceLine) {}
    bool next(string& tokenText) {
        while (position < inputLine.size()) {
            char character = inputLine[position];
            if (beginsNumber(inputLine, position) ||
                ((character == '+' || character == '-') &&
                 beginsNumber(inputLine, position + 1))) {
                break;
            }
            ++position;
        }
        if (position == inputLine.size()) {
            return false;
        }
        auto startPosition = position;
        if (inputLine[position] == '+' || inputLine[position] == '-') {
            ++position;
        }
        while (position < inputLine.size() &&
               (isDigit(inputLine[position]) || inputLine[position] == '.')) {
            ++position;
        }
        tokenText = inputLine.substr(startPosition, position - startPosition);
        return true;
    }
};
} // namespace detail

// 有號大整數：小端序 base-100 limbs，符號與數值分開保存。
class BigInteger {
private:
    u32string limbs;
    bool negative = false;

    void normalize() {
        while (limbs.size() > 1 && limbs.back() == 0) {
            limbs.pop_back();
        }
        if (limbs.empty()) {
            limbs.push_back(0);
        }
        if (isZero()) {
            negative = false;
        }
    }
    BigInteger addMagnitude(BigInteger left, BigInteger right) {
        BigInteger result;
        auto digitCount = max(left.limbs.size(), right.limbs.size());
        result.limbs.assign(digitCount, 0);
        unsigned int carryValue = 0;
        for (detail::Size digitIndex = 0; digitIndex < digitCount; ++digitIndex) {
            unsigned int digitValue = carryValue;
            if (digitIndex < left.limbs.size()) {
                digitValue += static_cast<unsigned int>(left.limbs.at(digitIndex));
            }
            if (digitIndex < right.limbs.size()) {
                digitValue += static_cast<unsigned int>(right.limbs.at(digitIndex));
            }
            result.limbs.at(digitIndex) = static_cast<char32_t>(digitValue % 100);
            carryValue = digitValue / 100;
        }
        if (carryValue != 0) {
            result.limbs.push_back(static_cast<char32_t>(carryValue));
        }
        result.normalize();
        return result;
    }    // 前提：|left| >= |right|。
    BigInteger subtractMagnitude(BigInteger left, BigInteger right) {
        BigInteger result;
        result.limbs.assign(left.limbs.size(), 0);
        int borrowValue = 0;
        for (detail::Size digitIndex = 0; digitIndex < left.limbs.size(); ++digitIndex) {
            int digitValue = static_cast<int>(left.limbs.at(digitIndex)) - borrowValue;
            if (digitIndex < right.limbs.size()) {
                digitValue -= static_cast<int>(right.limbs.at(digitIndex));
            }
            borrowValue = digitValue < 0 ? 1 : 0;
            if (digitValue < 0) {
                digitValue += static_cast<int>(100);
            }
            result.limbs.at(digitIndex) = static_cast<char32_t>(digitValue);
        }
        result.normalize();
        return result;
    }
    BigInteger slice(detail::Size startPosition, detail::Size digitCount) {
        if (startPosition >= limbs.size() || digitCount == 0) {
            return BigInteger();
        }
        BigInteger result;
        result.limbs = limbs.substr(startPosition, digitCount);
        result.normalize();
        return result;
    }
    BigInteger schoolMultiply(BigInteger left, BigInteger right) {
        if (left.isZero() || right.isZero()) {
            return BigInteger();
        }
        BigInteger result;
        result.limbs.assign(left.limbs.size() + right.limbs.size(), 0);
        for (detail::Size leftIndex = 0; leftIndex < left.limbs.size(); ++leftIndex) {
            unsigned long long carryValue = 0;
            for (detail::Size rightIndex = 0; rightIndex < right.limbs.size(); ++rightIndex) {
                auto resultIndex = leftIndex + rightIndex;
                unsigned long long digitValue =
                    static_cast<unsigned long long>(left.limbs.at(leftIndex)) *
                    static_cast<unsigned long long>(right.limbs.at(rightIndex)) +
                    static_cast<unsigned long long>(result.limbs.at(resultIndex)) + carryValue;
                result.limbs.at(resultIndex) = static_cast<char32_t>(digitValue % 100);
                carryValue = digitValue / 100;
            }
            auto resultIndex = leftIndex + right.limbs.size();
            while (carryValue != 0) {
                if (resultIndex == result.limbs.size()) {
                    result.limbs.push_back(0);
                }
                unsigned long long digitValue =
                    static_cast<unsigned long long>(result.limbs.at(resultIndex)) + carryValue;
                result.limbs.at(resultIndex) = static_cast<char32_t>(digitValue % 100);
                carryValue = digitValue / 100;
                ++resultIndex;
            }
        }
        result.normalize();
        return result;
    }
    BigInteger karatsubaMultiply(BigInteger left, BigInteger right) {
        if (left.isZero() || right.isZero()) {
            return BigInteger();
        }
        if (min(left.limbs.size(), right.limbs.size()) <= 32) {
            return schoolMultiply(left, right);
        }
        auto splitCount = max(left.limbs.size(), right.limbs.size()) / 2;
        BigInteger leftLow = left.slice(0, splitCount);
        BigInteger leftHigh = left.slice(splitCount, left.limbs.size());
        BigInteger rightLow = right.slice(0, splitCount);
        BigInteger rightHigh = right.slice(splitCount, right.limbs.size());
        BigInteger lowProduct = karatsubaMultiply(leftLow, rightLow);
        BigInteger highProduct = karatsubaMultiply(leftHigh, rightHigh);
        BigInteger middleProduct = karatsubaMultiply(
            addMagnitude(leftLow, leftHigh), addMagnitude(rightLow, rightHigh)) - lowProduct - highProduct;
        return lowProduct + middleProduct.shiftBase(splitCount) +
               highProduct.shiftBase(splitCount * 2);
    }
public:
    BigInteger(long long value = 0) {
        negative = value < 0;
        unsigned long long magnitude = value < 0
            ? static_cast<unsigned long long>(-(value + 1)) + 1
            : static_cast<unsigned long long>(value);
        do {
            limbs.push_back(static_cast<char32_t>(magnitude % 100));
            magnitude /= 100;
        } while (magnitude != 0);
        normalize();
    }
    BigInteger(string decimalDigits, bool isNegative = false) {
        auto firstDigit = decimalDigits.find_first_not_of('0');
        if (firstDigit == string::npos) {
            limbs.push_back(0);
            return;
        }
        limbs.clear();
        auto endPosition = decimalDigits.size();
        while (endPosition > firstDigit) {
            auto startPosition = endPosition - firstDigit >= 2 ? endPosition - 2 : firstDigit;
            unsigned int digitValue = 0;
            for (auto digitIndex = startPosition; digitIndex < endPosition; ++digitIndex) {
                if (!detail::isDigit(decimalDigits[digitIndex])) {
                    throw 0;
                }
                digitValue = digitValue * 10 + static_cast<unsigned int>(decimalDigits[digitIndex] - '0');
            }
            limbs.push_back(static_cast<char32_t>(digitValue));
            endPosition = startPosition;
        }
        negative = isNegative;
        normalize();
    }
    BigInteger(u32string sourceLimbs, bool isNegative) {
        limbs = sourceLimbs;
        for (char32_t digitValue : limbs) {
            if (digitValue >= 100) {
                throw 0;
            }
        }
        negative = isNegative;
        normalize();
    }
    bool isZero() { return limbs.size() == 1 && limbs.front() == 0; }
    bool isNegative() { return negative; }
    bool isOdd() { return limbs.front() % 2 != 0; }
    bool isMagnitudeOne() { return limbs.size() == 1 && limbs.front() == 1; }
    u32string& packedLimbs() { return limbs; }
    int compareMagnitude(BigInteger other) {
        if (limbs.size() != other.limbs.size()) {
            return limbs.size() < other.limbs.size() ? -1 : 1;
        }
        for (auto digitIndex = limbs.size(); digitIndex > 0; --digitIndex) {
            if (limbs.at(digitIndex - 1) != other.limbs.at(digitIndex - 1)) {
                return limbs.at(digitIndex - 1) < other.limbs.at(digitIndex - 1) ? -1 : 1;
            }
        }
        return 0;
    }
    bool operator==(BigInteger other) {
        return negative == other.negative && limbs == other.limbs;
    }
    BigInteger operator-() {
        BigInteger result = *this;
        if (!result.isZero()) {
            result.negative = !result.negative;
        }
        return result;
    }
    BigInteger operator+(BigInteger other) {
        BigInteger result;
        if (negative == other.negative) {
            result = addMagnitude(*this, other);
            result.negative = negative;
        } else {
            int comparison = compareMagnitude(other);
            if (comparison >= 0) {
                result = subtractMagnitude(*this, other);
                result.negative = negative;
            } else {
                result = subtractMagnitude(other, *this);
                result.negative = other.negative;
            }
        }
        result.normalize();
        return result;
    }
    BigInteger operator-(BigInteger other) {
        return *this + (-other);
    }
    BigInteger operator*(BigInteger other) {
        BigInteger result = karatsubaMultiply(*this, other);
        result.negative = negative != other.negative && !result.isZero();
        return result;
    }
    BigInteger shiftBase(detail::Size groupCount) {
        if (isZero()) {
            return BigInteger();
        }
        BigInteger result = *this;
        result.limbs.insert(0, groupCount, 0);
        return result;
    }
    BigInteger dropLowGroups(detail::Size groupCount) {
        BigInteger result = *this;
        result.limbs.erase(0, min(groupCount, result.limbs.size()));
        result.normalize();
        return result;
    }
    detail::Size lowZeroGroups(detail::Size maxCount) {
        detail::Size zeroCount = 0;
        while (zeroCount < maxCount && zeroCount < limbs.size() && limbs.at(zeroCount) == 0) {
            ++zeroCount;
        }
        return zeroCount;
    }
    bool isDivisibleBy100() {
        return limbs.front() == 0;
    }
    BigInteger roundedDivideBase(detail::Size groupCount) {
        if (groupCount == 0) {
            return *this;
        }
        BigInteger result = dropLowGroups(groupCount);
        result.negative = false;
        if (groupCount <= limbs.size() && limbs.at(groupCount - 1) >= 50) {
            result = addMagnitude(result, BigInteger(1));
        }
        result.negative = negative && !result.isZero();
        return result;
    }
    void halve() {
        unsigned int remainderValue = 0;
        for (auto digitIndex = limbs.size(); digitIndex > 0; --digitIndex) {
            unsigned int digitValue = remainderValue * 100 +
                static_cast<unsigned int>(limbs.at(digitIndex - 1));
            limbs.at(digitIndex - 1) = static_cast<char32_t>(digitValue / 2);
            remainderValue = digitValue % 2;
        }
        normalize();
    }
    int toBoundedInt(int upperBound) {
        if (negative) {
            throw 0;
        }
        unsigned long long resultValue = 0;
        for (auto digitIndex = limbs.size(); digitIndex > 0; --digitIndex) {
            resultValue = resultValue * 100 +
                static_cast<unsigned long long>(limbs.at(digitIndex - 1));
            if (resultValue > static_cast<unsigned long long>(upperBound)) {
                throw 0;
            }
        }
        return static_cast<int>(resultValue);
    }
    string toString() {
        string resultText = negative ? "-" : "";
        auto firstValue = static_cast<unsigned int>(limbs.back());
        resultText += to_string(firstValue);
        for (auto digitIndex = limbs.size() - 1; digitIndex > 0; --digitIndex) {
            auto digitValue = static_cast<unsigned int>(limbs.at(digitIndex - 1));
            resultText.push_back(static_cast<char>('0' + digitValue / 10));
            resultText.push_back(static_cast<char>('0' + digitValue % 10));
        }
        return resultText;
    }
};
class Fixed2 {// 所有對外數字固定以「數值 * 100」的有號大整數保存。
    BigInteger scaledValue;
public:
    Fixed2() = default;
    Fixed2(BigInteger sourceValue) : scaledValue(sourceValue) {}

    Fixed2(string tokenText) {
        auto position = string::size_type{0};
        bool isNegative = false;
        if (!tokenText.empty() && (tokenText.front() == '+' || tokenText.front() == '-')) {
            isNegative = tokenText.front() == '-';
            position = 1;
        }
        string integerDigits;
        string fractionalDigits;
        bool hasDecimalPoint = false;
        for (; position < tokenText.size(); ++position) {
            char character = tokenText[position];
            if (character == '.' && !hasDecimalPoint) {
                hasDecimalPoint = true;
            } else if (detail::isDigit(character)) {
                (hasDecimalPoint ? fractionalDigits : integerDigits).push_back(character);
            } else {
                throw 0;
            }
        }
        if (integerDigits.empty() && fractionalDigits.empty()) {
            throw 0;
        }
        if (fractionalDigits.size() > 2) fractionalDigits.resize(2);
        while (fractionalDigits.size() < 2) {
            fractionalDigits.push_back('0');
        }
        scaledValue = BigInteger(integerDigits + fractionalDigits, isNegative);
    }

    BigInteger& units() { return scaledValue; }
    bool isZero() { return scaledValue.isZero(); }
    bool isNegative() { return scaledValue.isNegative(); }
    bool isMagnitudeOne() { return scaledValue.compareMagnitude(BigInteger(100)) == 0; }

    BigInteger toExponent() {
        if (scaledValue.isNegative()) {
            throw 0;
        }
        if (!scaledValue.isDivisibleBy100()) {
            throw 0;
        }
        return scaledValue.dropLowGroups(1);
    }
    string toString() {
        string resultText = scaledValue.toString();
        auto digitStart = resultText.front() == '-' ? string::size_type{1} : string::size_type{0};
        while (resultText.size() - digitStart < 3) {
            resultText.insert(digitStart, 1, '0');
        }
        resultText.insert(resultText.size() - 2, 1, '.');
        while (resultText.back() == '0') resultText.pop_back();
        if (resultText.back() == '.') resultText.pop_back();
        return resultText;
    }
    Fixed2(float value) {
        if (!isfinite(value)) {
            throw 0;
        }
        ostringstream formattedValue;
        formattedValue.setf(ios::fixed, ios::floatfield);
        formattedValue.precision(2);
        formattedValue << value;
        scaledValue = Fixed2(formattedValue.str()).scaledValue;
        istringstream inputCheck(toString());
        float checkedValue = 0;
        if (!(inputCheck >> checkedValue) || checkedValue != value) {
            throw 0;
        }
    }

    float toFloat() {
        istringstream inputValue(toString());
        float resultValue = 0;
        if (!(inputValue >> resultValue) || !isfinite(resultValue) ||
            !(Fixed2(resultValue).units() == scaledValue)) {
            throw 0;
        }
        return resultValue;
    }
};

BigInteger parseExponent(string tokenText) {
    auto decimalPosition = tokenText.find('.');
    if (decimalPosition != string::npos) {
        for (auto position = decimalPosition + 1; position < tokenText.size(); ++position) {
            if (!detail::isDigit(tokenText[position])) throw 0;
        }
        if (tokenText.front() == '-' && tokenText.find_first_not_of("-0.") != string::npos) throw 0;
        tokenText.erase(decimalPosition);
        if (tokenText.empty() || tokenText == "+" || tokenText == "-") tokenText += '0';
    }
    return Fixed2(tokenText).toExponent();
}// 代入時使用精確的 units / 100^scaleGroups，整個 Eval 完成後才四捨五入。
class ExactDecimal {
private:
    BigInteger units;
    detail::Size scaleGroups = 0;

    void normalize() {
        if (units.isZero()) {
            scaleGroups = 0;
            return;
        }
        auto removableGroups = units.lowZeroGroups(scaleGroups);
        units = units.dropLowGroups(removableGroups);
        scaleGroups -= removableGroups;
    }

    ExactDecimal(BigInteger sourceUnits, detail::Size sourceScale)
        : units(sourceUnits), scaleGroups(sourceScale) {
        normalize();
    }

public:
    ExactDecimal() = default;
    ExactDecimal(Fixed2 value) : units(value.units()), scaleGroups(1) {
        normalize();
    }

    ExactDecimal operator+(ExactDecimal other) {
        auto commonScale = max(scaleGroups, other.scaleGroups);
        return ExactDecimal(units.shiftBase(commonScale - scaleGroups) +
            other.units.shiftBase(commonScale - other.scaleGroups), commonScale);
    }

    ExactDecimal operator*(ExactDecimal other) {
        if (units.isZero() || other.units.isZero()) {
            return ExactDecimal();
        }
        return ExactDecimal(units * other.units, scaleGroups + other.scaleGroups);
    }

    ExactDecimal power(BigInteger exponent) {
        ExactDecimal result(Fixed2("1"));
        if (exponent.isZero()) {
            return result;
        }
        if (units.isZero()) {
            return ExactDecimal();
        }
        if (scaleGroups == 0 && units.isMagnitudeOne()) {
            return units.isNegative() && exponent.isOdd()
                ? ExactDecimal(Fixed2("-1")) : result;
        }
        ExactDecimal baseValue = *this;
        while (!exponent.isZero()) {
            if (exponent.isOdd()) {
                result = result * baseValue;
            }
            exponent.halve();
            if (!exponent.isZero()) {
                baseValue = baseValue * baseValue;
            }
        }
        return result;
    }
    Fixed2 rounded() {
        if (scaleGroups == 0) {
            return Fixed2(units.shiftBase(1));
        }
        return Fixed2(units.roundedDivideBase(scaleGroups - 1));
    }
};
class Polynomial;// 可變長度項目以兩個 u32string 管理：offsets 是索引，data 保存大數 limbs。
class DecimalPolynomial {
    friend class Polynomial;
private:
    u32string data;
    u32string offsets;
    detail::Size termCount() {
        return offsets.size() / 2;
    }
    detail::Size recordStart(detail::Size termIndex) {
        return detail::readSize(offsets, termIndex * 2);
    }

    detail::Size exponentLength(detail::Size termIndex) {
        return detail::readSize(data, recordStart(termIndex));
    }

    BigInteger exponentAt(detail::Size termIndex) {
        auto startPosition = recordStart(termIndex);
        return BigInteger(data.substr(startPosition + 5, exponentLength(termIndex)), false);
    }

    BigInteger coefficientAt(detail::Size termIndex) {
        auto startPosition = recordStart(termIndex);
        return BigInteger(data.substr(startPosition + 5 + exponentLength(termIndex),
            detail::readSize(data, startPosition + 2)), data.at(startPosition + 4) != 0);
    }

    int compareExponents(DecimalPolynomial& left, detail::Size leftIndex,
                         DecimalPolynomial& right, detail::Size rightIndex) {
        auto leftLength = left.exponentLength(leftIndex);
        auto rightLength = right.exponentLength(rightIndex);
        if (leftLength != rightLength) {
            return leftLength < rightLength ? -1 : 1;
        }
        auto leftStart = left.recordStart(leftIndex) + 5;
        auto rightStart = right.recordStart(rightIndex) + 5;
        for (auto digitIndex = leftLength; digitIndex > 0; --digitIndex) {
            auto leftDigit = left.data.at(leftStart + digitIndex - 1);
            auto rightDigit = right.data.at(rightStart + digitIndex - 1);
            if (leftDigit != rightDigit) {
                return leftDigit < rightDigit ? -1 : 1;
            }
        }
        return 0;
    }

    void append(BigInteger coefficient, BigInteger exponent) {
        if (coefficient.isZero()) {
            return;
        }
        if (exponent.isNegative()) {
            throw 0;
        }
        auto exponentSize = exponent.packedLimbs().size();
        auto coefficientSize = coefficient.packedLimbs().size();
        auto cellCount = 5 + exponentSize + coefficientSize;
        auto startPosition = data.size();
        data.reserve(startPosition + cellCount);
        offsets.reserve(offsets.size() + 2);
        detail::appendSize(data, exponentSize);
        detail::appendSize(data, coefficientSize);
        data.push_back(coefficient.isNegative() ? 1 : 0);
        data += exponent.packedLimbs();
        data += coefficient.packedLimbs();
        detail::appendSize(offsets, startPosition);
    }

    void normalize(detail::Size roundingGroups) {
        u32string order;
        order.reserve(termCount() * 2);
        for (detail::Size termIndex = 0; termIndex < termCount(); ++termIndex) {
            detail::appendSize(order, termIndex);
        }
        u32string buffer(order.size(), 0);
        for (detail::Size width = 1; width < termCount(); width *= 2) {
            for (detail::Size start = 0; start < termCount(); start += width * 2) {
                auto left = start;
                auto middle = min(start + width, termCount());
                auto right = middle;
                auto end = min(middle + width, termCount());
                for (auto position = start; position < end; ++position) {
                    if (right == end || (left < middle && compareExponents(*this,
                        detail::readSize(order, left * 2), *this,
                        detail::readSize(order, right * 2)) >= 0)) {
                        detail::writeSize(buffer, position * 2, detail::readSize(order, left++ * 2));
                    } else {
                        detail::writeSize(buffer, position * 2, detail::readSize(order, right++ * 2));
                    }
                }
            }
            order.swap(buffer);
        }
        DecimalPolynomial normalized;
        detail::Size position = 0;
        while (position < termCount()) {
            auto firstIndex = detail::readSize(order, position * 2);
            BigInteger coefficient;
            do {
                coefficient = coefficient + coefficientAt(detail::readSize(order, position * 2));
                ++position;
            } while (position < termCount() && compareExponents(*this, firstIndex,
                *this, detail::readSize(order, position * 2)) == 0);
            normalized.append(coefficient.roundedDivideBase(roundingGroups), exponentAt(firstIndex));
        }
        data.swap(normalized.data);
        offsets.swap(normalized.offsets);
    }
public:
    DecimalPolynomial() = default;
    DecimalPolynomial(string inputLine) {
        detail::NumberScanner scanner(inputLine);
        string tokenText;
        if (!scanner.next(tokenText)) {
            throw 0;
        }
        if (tokenText.find('.') != string::npos) {
            throw 0;
        }
        BigInteger inputCount = Fixed2(tokenText).toExponent();
        BigInteger actualCount;
        while (scanner.next(tokenText)) {
            Fixed2 coefficient = Fixed2(tokenText);
            if (!scanner.next(tokenText)) {
                throw 0;
            }
            append(coefficient.units(), parseExponent(tokenText));
            actualCount = actualCount + BigInteger(1);
        }
        if (!(actualCount == inputCount)) {
            throw 0;
        }
        normalize(0);
    }
    DecimalPolynomial Add(DecimalPolynomial poly) {
        DecimalPolynomial result;
        detail::Size leftIndex = 0;
        detail::Size rightIndex = 0;
        while (leftIndex < termCount() || rightIndex < poly.termCount()) {
            int comparison = rightIndex == poly.termCount() ? 1 :
                (leftIndex == termCount() ? -1 : compareExponents(*this, leftIndex, poly, rightIndex));
            if (comparison > 0) {
                result.append(coefficientAt(leftIndex), exponentAt(leftIndex));
                ++leftIndex;
            } else if (comparison < 0) {
                result.append(poly.coefficientAt(rightIndex), poly.exponentAt(rightIndex));
                ++rightIndex;
            } else {
                result.append(coefficientAt(leftIndex) + poly.coefficientAt(rightIndex), exponentAt(leftIndex));
                ++leftIndex;
                ++rightIndex;
            }
        }
        return result;
    }
    DecimalPolynomial Mult(DecimalPolynomial poly) {
        DecimalPolynomial result;
        for (detail::Size leftIndex = 0; leftIndex < termCount(); ++leftIndex) {
            for (detail::Size rightIndex = 0; rightIndex < poly.termCount(); ++rightIndex) {
                result.append(coefficientAt(leftIndex) * poly.coefficientAt(rightIndex),
                    exponentAt(leftIndex) + poly.exponentAt(rightIndex));
            }
        }// 原始乘積單位是 1/10000；同次項先合併，再除以 100 四捨五入。
        result.normalize(1);
        return result;
    }

    Fixed2 Eval(Fixed2 x) {
        ExactDecimal baseValue(x);
        ExactDecimal result;
        for (detail::Size termIndex = 0; termIndex < termCount(); ++termIndex) {
            result = result + ExactDecimal(Fixed2(coefficientAt(termIndex))) *
                baseValue.power(exponentAt(termIndex));
        }
        return result.rounded();
    }

    friend istream& operator>>(istream& input, DecimalPolynomial& poly) {
        string inputLine;
        if (!getline(input, inputLine)) {
            return input;
        }
        try {
            DecimalPolynomial candidate(inputLine);
            poly.data.swap(candidate.data);
            poly.offsets.swap(candidate.offsets);
        } catch (int) {
            input.setstate(ios::failbit);
            throw;
        }
        return input;
    }

    friend ostream& operator<<(ostream& output, DecimalPolynomial poly) {
        if (poly.termCount() == 0) {
            return output << 0;
        }
        for (detail::Size termIndex = 0; termIndex < poly.termCount(); ++termIndex) {
            Fixed2 coefficient(poly.coefficientAt(termIndex));
            BigInteger exponent = poly.exponentAt(termIndex);
            if (termIndex == 0) {
                if (coefficient.isNegative()) {
                    output << '-';
                }
            } else {
                output << (coefficient.isNegative() ? " - " : " + ");
            }
            if (exponent.isZero() || !coefficient.isMagnitudeOne()) {
                string coefficientText = coefficient.toString();
                output << (coefficient.isNegative() ? coefficientText.substr(1) : coefficientText);
            }
            if (!exponent.isZero()) {
                output << 'x';
                if (!(exponent == BigInteger(1))) {
                    output << '^' << exponent.toString();
                }
            }
        }
        return output;
    }
};

class Term {
    friend class Polynomial;
private:
    float coef;
    int exp;
};
class Polynomial {
private:
    Term* termArray;
    int capacity;
    int terms;

    DecimalPolynomial toDecimal() {
        DecimalPolynomial result;
        for (int termIndex = 0; termIndex < terms; ++termIndex) {
            result.append(Fixed2(termArray[termIndex].coef).units(), BigInteger(termArray[termIndex].exp));
        }
        return result;
    }

    Polynomial(DecimalPolynomial source) : Polynomial() {
        if (source.termCount() > 2147483647) throw 0;
        capacity = terms = static_cast<int>(source.termCount());
        termArray = new Term[capacity];
        for (int termIndex = 0; termIndex < terms; ++termIndex) {
            termArray[termIndex].exp = source.exponentAt(termIndex).toBoundedInt(2147483647);
            termArray[termIndex].coef = Fixed2(source.coefficientAt(termIndex)).toFloat();
        }
    }
public:
    Polynomial() : termArray(nullptr), capacity(0), terms(0) {}
    Polynomial(Polynomial& other) : Polynomial() {
        capacity = terms = other.terms;
        termArray = new Term[capacity];
        for (int termIndex = 0; termIndex < terms; ++termIndex) {
            termArray[termIndex] = other.termArray[termIndex];
        }
    }
    ~Polynomial() { delete[] termArray; }
    Polynomial& operator=(Polynomial other) {
        swap(termArray, other.termArray);
        swap(capacity, other.capacity);
        swap(terms, other.terms);
        return *this;
    }
    Polynomial Add(Polynomial poly) { return Polynomial(toDecimal().Add(poly.toDecimal())); }
    Polynomial Mult(Polynomial poly) { return Polynomial(toDecimal().Mult(poly.toDecimal())); }
    float Eval(float f) { return toDecimal().Eval(Fixed2(f)).toFloat(); }
    friend istream& operator>>(istream& input, Polynomial& poly) {
        DecimalPolynomial parsed;
        if (!(input >> parsed)) return input;
        try {
            Polynomial candidate(parsed);
            poly = candidate;
        } catch (int) {
            input.setstate(ios::failbit);
            throw;
        }
        return input;
    }
    friend ostream& operator<<(ostream& output, Polynomial poly) {
        return output << poly.toDecimal();
    }
};

Fixed2 evaluateForMenu(DecimalPolynomial& poly, Fixed2 x) {
    return poly.Eval(x);
}

Fixed2 evaluateForMenu(Polynomial& poly, Fixed2 x) {
    return Fixed2(poly.Eval(x.toFloat()));
}

namespace detail {
Fixed2 parseSingleNumber(string inputLine) {
    NumberScanner scanner(inputLine);
    string tokenText;
    if (!scanner.next(tokenText)) {
        throw 0;
    }
    Fixed2 result = Fixed2(tokenText);
    if (scanner.next(tokenText)) {
        throw 0;
    }
    return result;
}

int parseChoice(string inputLine) {
    istringstream command(inputLine);
    string token, extra;
    if (!(command >> token) || command >> extra || token.size() != 1 ||
        token[0] < '1' || token[0] > '7') {
        return 0;
    }
    return token[0] - '0';
}

template <class Poly>
bool readPolynomialLine(istream& input, ostream& output,
                        string prompt, Poly& poly) {
    while (true) {
        output << prompt;
        try {
            return static_cast<bool>(input >> poly);
        } catch (int) {
            output << "輸入錯誤請重新輸入\n";
            if (input.eof()) {
                return false;
            }
            input.clear();
        }
    }
}
bool readValueLine(istream& input, ostream& output, Fixed2& value) {
    while (true) {
        output << "請輸入 x：";
        string inputLine;
        if (!getline(input, inputLine)) {
            return false;
        }
        try {
            value = parseSingleNumber(inputLine);
            return true;
        } catch (int) {
            output << "輸入錯誤請重新輸入\n";
        }
    }
}
} // namespace detail
int main() {
    cout << "輸入得在同一行第一個數字是整數項數，之後依序是係數(只接受到小數點後兩位多超出部分捨去)、指數（非負，小數部分捨去）。\n"
           << "係數與 x 只取小數點後兩位；結果四捨五入到兩位，省略尾端的 0，係數的正負號只接受最靠近數字的那個如-+1 = +1 反之亦然。\n"
           << "格式：項數 係數0 指數0 係數1 指數1 ...；例如 2 1 2 3.0 4.0。 若輸入為a2a1.000000a02a3.00a4.0001a是等價於左側例子的。\n"
           << "正負號保留，其他符號只當分隔；輸入 0 表示零多項式。\n\n";
    DecimalPolynomial polyA;
    DecimalPolynomial polyB;
    if (!detail::readPolynomialLine(cin, cout, "請輸入 A：", polyA) ||
        !detail::readPolynomialLine(cin, cout, "請輸入 B：", polyB)) {
        cout << "\n輸入結束。\n";
        return 0;
    }
    while (true) {
        try {
            cout << "\n==================選單==================\n1. 顯示 A、B\n2. A + B\n3. A * B\n4. 計算 A(x)\n5. 計算 B(x)\n6. 重新輸入 A\n7. 重新輸入 B\n其他輸入：結束\n\n請選擇 1–7（其他輸入結束）：";
            string inputLine;
            if (!getline(cin, inputLine)) {
                cout << "\n輸入結束。\n";
                return 0;
            }
            int choice = detail::parseChoice(inputLine);
            if (choice == 0) {
                cout << "程式結束。\n";
                return 0;
            }
            if (choice == 1) {
                cout << "A = " << polyA << "\nB = " << polyB << '\n';
            } else if (choice == 2) {
                DecimalPolynomial result = polyA.Add(polyB);
                cout << "A + B = " << result << '\n';
            } else if (choice == 3) {
                DecimalPolynomial result = polyA.Mult(polyB);
                cout << "A * B = " << result << '\n';
            } else if (choice == 4 || choice == 5) {
                Fixed2 x;
                if (!detail::readValueLine(cin, cout, x)) {
                    cout << "\n輸入結束。\n";
                    return 0;
                }
                Fixed2 result = choice == 4 ? evaluateForMenu(polyA, x) : evaluateForMenu(polyB, x);
                cout << (choice == 4 ? "A(" : "B(") << x.toString() << ") = " << result.toString() << '\n';
            } else {
                bool complete = choice == 6? detail::readPolynomialLine(cin, cout, "請重新輸入 A：", polyA): detail::readPolynomialLine(cin, cout, "請重新輸入 B：", polyB);
                if (!complete) {
                    cout << "\n輸入結束。\n";
                    return 0;
                }
                cout << "已更新 " << (choice == 6 ? 'A' : 'B') << "。\n";
            }
        } catch (int) {
            cout << "指令格式或輸入錯誤\n";
            if (cin.eof()) {
                cout << "輸入結束。\n";
                return 0;
            }
            cin.clear();
        }
    }
}


```

### 時間複雜度

令：

- \(m,n\)：兩個多項式的非零項數。
- \(k=mn\)：相乘時最多產生的未合併項數。
- \(d\)：大整數的 100 進位群組數。
- \(e\)：指數的大整數群組數。
- \(E\)：指數的數值。

兩個多項式已依指數排序時，加法的雙索引主迴圈最多掃描 \(m+n\) 項，因此項目處理次數為：

\[
O(m+n)
\]

但每次指數比較、係數相加及資料複製，仍需依大數長度計算成本，不能一律視為固定時間。

多項式乘法先產生 \(mn\) 個配對，之後對最多 \(k\) 項排序，主要需要 \(k\) 次係數乘法及指數相加，以及：

\[
O(k\log k)
\]

次指數比較。一次指數比較最壞需掃描 \(O(e)\) 個群組。

對長度相近、各有 \(d\) 個群組的大整數，卡拉楚巴的遞迴關係為：

\[
T(d)=3T(d/2)+O(d)
\]

因此主要算術時間複雜度為：

\[
\boxed{O(d^{\log_2 3})\approx O(d^{1.585})}
\]

快速冪需要 \(O(\log(E+1))\) 次量級的大數乘法，但運算期間數字會增長，指數除以 2 也需掃描群組，因此不代表整個代入只需要 \(O(\log E)\) 時間。

### 主要操作時間複雜度

| 操作 | 時間複雜度或主要步驟 | 說明 |
|---|---|---|
| 大整數比較、加法、減法 | \(O(d)\) | 最壞需掃描各群組 |
| 一般大整數乘法 | \(O(d_1d_2)\) | 兩個運算元的群組數分別為 \(d_1,d_2\) |
| 平衡長度卡拉楚巴乘法 | \(O(d^{1.585})\) | 小數字改用一般乘法 |
| 指數比較 | \(O(e)\) | 先比較長度，再由高位比較 |
| t 項正規化排序 | \(O(t\log t)\) 次比較 | 另需合併係數及重建資料 |
| 多項式加法 | \(O(m+n)\) 項掃描 | 另計大數操作及複製 |
| 多項式乘法 | \(mn\) 次配對及 \(O(k\log k)\) 次比較 | 係數乘法使用大數演算法 |
| 單項快速冪 | \(O(\log(E+1))\) 次量級的乘法 | 每次成本依中間值長度而定 |

程式有值傳遞、字串切片、配置及複製成本，表中的項目掃描次數與比較次數不等於整個程式的完整執行時間。

### 空間複雜度

一個有 \(d\) 個群組的大整數需要 \(O(d)\) 空間。若第 i 項的係數及指數分別有 \(d_i,e_i\) 個群組，則多項式主要儲存量為：

\[
O\left(\sum_i(d_i+e_i+1)\right)
\]

乘法最多先保存 \(k=mn\) 個未合併項目。若係數與指數的群組數上界分別為 \(d,e\)，這部分需要：

\[
\boxed{O(mn(d+e))}
\]

空間，排序索引與緩衝另需 \(O(mn)\)。這裡的 \(d\) 應包含當時乘積及合併係數的長度，不只計算最初輸入位數。

卡拉楚巴的平衡遞迴深度為 \(O(\log d)\)，每層仍保存數字副本及暫存乘積，不能僅以遞迴層數代表全部額外空間。

代入時，尺度對齊與次方計算可能讓中間分子大幅增長，因此空間還取決於精確中間值的位數，不能只用多項式項數估算。

### 測試案例

下表列出代表案例，輸出欄省略反覆出現的提示文字與選單。

| 測試案例 | 輸入或操作 | 預期與實際結果 |
|---|---|---|
| 基本顯示 | A：`3 3 2 2 1 1 0`；B：`2 1 1 -1 0` | A=`3x^2 + 2x + 1`，B=`x - 1` |
| 加法 | 對上述 A、B 執行指令 `2` | `A + B = 3x^2 + 3x` |
| 乘法 | 對上述 A、B 執行指令 `3` | `A * B = 3x^3 - x^2 - x - 1` |
| 代入 | 對上述 A 執行指令 `4`，x=`2` | `A(2) = 17` |
| 零多項式 | A：`0` | `A = 0` |
| 同次項抵消 | A：`2 1 2 -1 2` | `A = 0` |
| 係數截斷 | A：`1 2.345 0` | `A = 2.34` |
| 負數截斷 | A：`1 -2.345 0` | `A = -2.34` |
| 指數截斷與文字分隔 | A：`3 1 2 3 4 5.10abc20.01` | `A = 5.1x^20 + 3x^4 + x^2` |
| 負指數 | A：`1 2 -0.001` | `輸入錯誤請重新輸入` |
| 格式錯誤 | A：`1 1..234 0` | `輸入錯誤請重新輸入` |
| 乘法中點捨入 | 常數 `0.01` 乘 `0.50` | `0.01` |
| 負數中點捨入 | 常數 `-0.01` 乘 `0.50` | `-0.01` |
| 乘法捨入為零 | 常數 `0.01` 乘 `0.49` | `0` |
| 完整代入後捨入 | `0.01x^2 + 0.01x`，x=`0.50` | `0.01` |
| 超過原生整數的指數 | A=B=`2.25x^18446744073709551616`，計算乘積 | `5.06x^36893488147419103232` |
| 其他選單輸入 | `abc`、空白行、`1.0`、`1 2` | 直接結束程式 |

### 編譯環境與執行指令

 CLion 2026.2.2 內附的 **GCC 15.2.0、CMake 4.3.1** 及 **C++20** 建置，建置時未出現明顯錯誤或是導致運行失敗的警告，並成功運行。

    下面是shell的指令
```shell
g++ -std=c++20 -O2 -o polynomial.exe main.cpp
.\polynomial.exe
```

例如輸入：

```text
3 3 2 2 1 1 0
2 1 1 -1 0
1
2
3
4
2
0
```

輸出如下，省略提示文字。

```text
A = 3x^2 + 2x + 1
B = x - 1
A + B = 3x^2 + 3x
A * B = 3x^3 - x^2 - x - 1
A(2) = 17
```
## 申論

本題除了多項式的加乘，也需要處理數值表示、輸入完整性與小數精度。若直接以原生整數保存係數，數值大小會受型別上限限制；若直接使用浮點數，十進位小數的表示與捨入時機也較難控制。因此本實作使用大整數保存數值乘 100 的結果。

輸入截斷和運算捨入有不同目的。輸入 `2.345` 時直接保留 `2.34`，代表程式接受的資料本身只有兩位小數；但 `0.01 × 0.50` 產生的 `0.005` 必須依運算規則捨入為 `0.01`。若把兩者混為同一種處理，結果便可能與規格不同。

`u32string` 在本程式中扮演可變長度數值陣列，沒有使用 `vector`。大數類別的儲存由標準容器管理，但標準容器仍會配置記憶體。題目相容類別則保留要求的 `Term*`，透過 `new[]`、`delete[]` 管理自己的陣列，並以深複製避免不同物件共用同一塊儲存空間。

卡拉楚巴改善的是長整數乘法的主要算術成本，稀疏多項式仍需要逐項配對，後續還有指數比較、排序及合併成本。因此不能把整個多項式乘法直接宣稱為 `O(d^1.585)`，必須同時考慮項數與數字位數，其實有想過使用FFT-NTT來實現但是代碼會變態長所以就放棄這個方案。

大指數可以用少量群組保存，並不代表一般底數的大次方能在有限時間內輕易算完。快速冪減少乘法次數，但精確中間值仍可能非常龐大。程式不額外設定資源上限，也不另外攔截記憶體不足等環境例外，所以理論上上限取決於電腦環境與運行的環境。

此外，多項式乘法完成後的係數已經捨入到兩位，之後對該結果代入，是計算捨入後的多項式。因此它可能與先保留未捨入乘積、代入後才一次捨入的結果不同，這是本程式兩位小數運算規則的結果。

## 結論

0. 注意到ISOC++ 11的標準中有一個std::u32string 可以用在string標頭裡面所以就用這個來充當vector<int>用。
1. 本實作完成多項式建構、加法、乘法、代入，並整理同次項與零項。
2. 大數核心以 `u32string` 保存 100 進位群組，支援超過原生整數範圍的係數與非負指數。
3. 輸入小數直接截到兩位，乘法先合併同次項再捨入，代入則在完整計算後才捨入。
4. 大整數乘法採卡拉楚巴，短數字使用一般乘法；多項式乘法仍逐項配對並排序合併。
5. 多項式與 x 必須一行完成；選單只接受 `1`～`7`，其他輸入直接結束。
6. 效能同時受到項數、數字位數及中間值大小影響；代碼使用的是-O2優化在編譯後的運行速度正常來說會相對預設-O0快一點。
