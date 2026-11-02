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

// 題目相容版：只有此類別管理題目指定的 Term 陣列。
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

