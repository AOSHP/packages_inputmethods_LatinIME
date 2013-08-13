/*
 * Copyright (C) 2013, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "suggest/core/dictionary/binary_dictionary_header_reading_utils.h"

#include <cctype>
#include <cstdlib>

#include "defines.h"
#include "suggest/core/dictionary/byte_array_utils.h"

namespace latinime {

const int BinaryDictionaryHeaderReadingUtils::MAX_OPTION_KEY_LENGTH = 256;
const int BinaryDictionaryHeaderReadingUtils::VERSION_2_HEADER_MAGIC_NUMBER_SIZE = 4;
const int BinaryDictionaryHeaderReadingUtils::VERSION_2_HEADER_DICTIONARY_VERSION_SIZE = 2;
const int BinaryDictionaryHeaderReadingUtils::VERSION_2_HEADER_FLAG_SIZE = 2;
const int BinaryDictionaryHeaderReadingUtils::VERSION_2_HEADER_SIZE_FIELD_SIZE = 4;

const BinaryDictionaryHeaderReadingUtils::DictionaryFlags
        BinaryDictionaryHeaderReadingUtils::NO_FLAGS = 0;
// Flags for special processing
// Those *must* match the flags in makedict (BinaryDictInputOutput#*_PROCESSING_FLAG) or
// something very bad (like, the apocalypse) will happen. Please update both at the same time.
const BinaryDictionaryHeaderReadingUtils::DictionaryFlags
        BinaryDictionaryHeaderReadingUtils::GERMAN_UMLAUT_PROCESSING_FLAG = 0x1;
const BinaryDictionaryHeaderReadingUtils::DictionaryFlags
        BinaryDictionaryHeaderReadingUtils::SUPPORTS_DYNAMIC_UPDATE_FLAG = 0x2;
const BinaryDictionaryHeaderReadingUtils::DictionaryFlags
        BinaryDictionaryHeaderReadingUtils::FRENCH_LIGATURE_PROCESSING_FLAG = 0x4;

/* static */ int BinaryDictionaryHeaderReadingUtils::getHeaderSize(const uint8_t *const dictBuf) {
    // See the format of the header in the comment in
    // BinaryDictionaryFormatUtils::detectFormatVersion()
    return ByteArrayUtils::readUint32(dictBuf, VERSION_2_HEADER_MAGIC_NUMBER_SIZE
            + VERSION_2_HEADER_DICTIONARY_VERSION_SIZE + VERSION_2_HEADER_FLAG_SIZE);
}

/* static */ BinaryDictionaryHeaderReadingUtils::DictionaryFlags
        BinaryDictionaryHeaderReadingUtils::getFlags(const uint8_t *const dictBuf) {
    return ByteArrayUtils::readUint16(dictBuf, VERSION_2_HEADER_MAGIC_NUMBER_SIZE
            + VERSION_2_HEADER_DICTIONARY_VERSION_SIZE);
}

// Returns if the key is found or not and reads the found value into outValue.
/* static */ bool BinaryDictionaryHeaderReadingUtils::readHeaderValue(const uint8_t *const dictBuf,
        const char *const key, int *outValue, const int outValueSize) {
    if (outValueSize <= 0) {
        return false;
    }
    const int headerSize = getHeaderSize(dictBuf);
    int pos = getHeaderOptionsPosition();
    if (pos == NOT_A_DICT_POS) {
        // The header doesn't have header options.
        return false;
    }
    while (pos < headerSize) {
        if(ByteArrayUtils::compareStringInBufferWithCharArray(
                dictBuf, key, headerSize - pos, &pos) == 0) {
            // The key was found.
            const int length = ByteArrayUtils::readStringAndAdvancePosition(
                    dictBuf, outValueSize, outValue, &pos);
            // Add a 0 terminator to the string.
            outValue[length < outValueSize ? length : outValueSize - 1] = '\0';
            return true;
        }
        ByteArrayUtils::advancePositionToBehindString(dictBuf, headerSize - pos, &pos);
    }
    // The key was not found.
    return false;
}

/* static */ int BinaryDictionaryHeaderReadingUtils::readHeaderValueInt(
        const uint8_t *const dictBuf, const char *const key) {
    const int bufferSize = LARGEST_INT_DIGIT_COUNT;
    int intBuffer[bufferSize];
    char charBuffer[bufferSize];
    if (!readHeaderValue(dictBuf, key, intBuffer, bufferSize)) {
        return S_INT_MIN;
    }
    for (int i = 0; i < bufferSize; ++i) {
        charBuffer[i] = intBuffer[i];
        if (charBuffer[i] == '0') {
            break;
        }
        if (!isdigit(charBuffer[i])) {
            // If not a number, return S_INT_MIN
            return S_INT_MIN;
        }
    }
    return atoi(charBuffer);
}

} // namespace latinime
