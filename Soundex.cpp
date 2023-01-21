#include "gmock/gmock.h"
#include <iostream>

int main(int argc, char** argv)
{
    testing::InitGoogleMock(&argc ,argv);
    return RUN_ALL_TESTS();
}

class Soundex {
public:
    static const size_t MaxCodeLenght{4};

    std::string  encode(const std::string& word) const {
        return zeroPad(upperFront(head(word)) + tail(encodeDigits(word)));
    }
    
private:
    std::string upperFront(const std::string& string) const {
        return std::string(1, static_cast<unsigned char>(toupper(string.front())));
    }

    std::string head(const std::string& word) const {
        return word.substr(0, 1);
    }

    std::string zeroPad(const std::string& word) const {
        auto zerosNeeded = MaxCodeLenght - word.length();
        return word + std::string(zerosNeeded, '0');
    }
    
    std::string encodeDigits(const std::string& word) const {
        std::string encoding;
        encodeHead(encoding, word);
        encodeTail(encoding, word);
        return encoding;
    }

    void encodeHead(std::string& encoding, const std::string& word) const {
        encoding += encodeDigit(word.front());
    }

    void encodeTail(std::string& encoding, const std::string& word) const {
        for (auto i = 1u; i < word.length(); i++) {
            if (!isComplete(encoding)) 
                encodeLetter(encoding, word[i] , word[i-1]);
        }
    }

    void encodeLetter(std::string& encoding, char letter, char lastLetter) const {
        auto digit = encodeDigit(letter);
        if (digit != NotADigit && (digit != lastDigit(encoding) || isVowel(lastLetter)))
            encoding += digit;
    }

    bool isVowel(char letter) const {
        return std::string("aeiouy").find(lower(letter)) != std::string::npos; 
    }

    bool isComplete(const std::string& encoding) const { 
        return encoding.length() == MaxCodeLenght; 
    }

    std::string lastDigit(const std::string& encoding ) const {
        if (encoding.empty()) 
            return NotADigit;
        return std::string(1, encoding.back());
    }
public:
    std::string encodeDigit(char letter) const {
        std::map<char, std::string> encodings {
            {'b', "1"}, {'f', "1"}, {'p', "1"}, {'v', "1"},
            {'c', "2"}, {'g', "2"}, {'j', "2"}, {'k', "2"}, {'q', "2"}, {'s', "2"}, {'x', "2"}, {'z', "2"}, 
            {'d', "3"}, {'t', "3"},
            {'l', "4"},
            {'m', "5"}, {'n', "5"},
            {'r', "6"}, 
        };
        auto it = encodings.find(lower(letter));
        return it == encodings.end() ?  NotADigit : it->second;
    }
private:
    const std::string NotADigit{"*"};

    char lower(char c) const {
        return std::tolower(static_cast<unsigned char>(c));
    }

    std::string tail(const std::string& word) const {
        return word.substr(1);
    }
};

using namespace testing;

class SoundexEncoding : public Test {
public:
    Soundex soundex;
};

TEST_F(SoundexEncoding, RetainSoleLetterOfOneLetterWord) { 
   ASSERT_THAT( soundex.encode("A"), Eq("A000") );
}

TEST_F(SoundexEncoding, PadsWithZeroToEnsureThreeDigits) {
    ASSERT_THAT(soundex.encode("I"), Eq("I000"));
}

TEST_F(SoundexEncoding, ReplaceConsonantsWithAppropriateDigit) {
    ASSERT_THAT(soundex.encode("Ax"), Eq("A200"));
}

TEST_F(SoundexEncoding, IgnoreNonAlphabetics) {
    ASSERT_THAT(soundex.encode("A#"), Eq("A000"));
}

TEST_F(SoundexEncoding, ReplaceMultipleConsonantsWithDigits) {
    ASSERT_THAT(soundex.encode("Acdl"), Eq("A234"));
}

TEST_F(SoundexEncoding, LimitLengthToFourCharacters) {
    ASSERT_THAT(soundex.encode("Dcdlb").length(), Eq(4u));
}

TEST_F(SoundexEncoding, IgnoreVowelsLikeLetters) {
    ASSERT_THAT(soundex.encode("BaAeEiIoOuUhHyYcdl"), Eq("B234"));
}

TEST_F(SoundexEncoding, CombineDuplicateEncoding) {
    ASSERT_THAT(soundex.encodeDigit('b'), Eq(soundex.encodeDigit('f')));
    ASSERT_THAT(soundex.encodeDigit('c'), Eq(soundex.encodeDigit('g')));
    ASSERT_THAT(soundex.encodeDigit('d'), Eq(soundex.encodeDigit('t')));

    ASSERT_THAT(soundex.encode("Abfcgdt"), Eq("A123"));
}

TEST_F(SoundexEncoding, UppercaseFirstLetter) {
    ASSERT_THAT(soundex.encode("abcd"), StartsWith("A"));
}

TEST_F(SoundexEncoding,  IgnoresCaseWhenEncodingConsonants) {
    ASSERT_THAT(soundex.encode("BCDL"), Eq(soundex.encode("Bcdl")));
}

TEST_F(SoundexEncoding,  CombinesDuplicateCodesWhen2ndLetterDuplicates1st) {
    ASSERT_THAT(soundex.encode("Bbcd"), Eq("B230"));
}

TEST_F(SoundexEncoding,  DoesNotCombineDuplicateEncodingsSeparatedByVowels) {
    ASSERT_THAT(soundex.encode("Jbob"), Eq("J110"));
}
