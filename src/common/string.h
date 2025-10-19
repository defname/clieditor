/* Copyright (C) 2025 defname
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file string.h
 * @brief A high-level UTF-8 aware string implementation.
 *
 * This module provides:
 * - **String**: A mutable UTF-8 string type that stores the C string and
 *   auxiliary data for efficient character index ↔ byte offset mapping.
 * - **StringView**: A lightweight, read-only view into an existing String.
 * - **StringIterator**: A simple iterator to traverse a String or StringView
 *   character by character.
 *
 * Designed for correctness, clarity, and full UTF-8 compliance.
 */

#ifndef STRING_H
#define STRING_H

#include <stdlib.h>
#include <stdbool.h>

#define STRING_GROW_FACTOR_NUM 3
#define STRING_GROW_FACTOR_DENUM 2
#define STRING_GROW(v) (((v) * STRING_GROW_FACTOR_NUM) / STRING_GROW_FACTOR_DENUM)

#define STRING_INITIAL_CAPACITY 16
#define STRING_INITIAL_MULTIBYTE_OFFSETS_CAPACITY 6



// This is a little helper struct to store information about the multibyte
// character in a string for fast string operations
typedef struct {
    size_t char_index;  //< index of the character after the multibyte char (not the bytes!)
    size_t byte_offset; //< byte offset of the following character
} MultibyteIndexHelper;

/**
 * @brief The String type.
 */
typedef struct _String {
    char *bytes;              //< pointer to the NULL terminated C string
    size_t char_count;        //< number of characters in the String
    size_t bytes_capacity;    //< capacity available for bytes

    MultibyteIndexHelper *multibytes;  //< information about the indices and byte offsets of the multibyte characters
    size_t multibytes_size;            //< number of entries in multibytes
    size_t multibytes_capacity;        //< total capacity available for multibytes
    bool multibytes_invalid;     //< true if recalulation is needed

    size_t bytes_size;        //< number of bytes until '\0' (basically strlen(bytes))
} String;


/**
 * @brief A View is a lightweight readonly part of a String.
 * 
 * It just points to an existing string and doesn't own the bytes itself.
 * So is must not be destructed like a String.
 */
typedef struct _StringView {
    const char *bytes;
    size_t bytes_size;
    size_t char_count;
} StringView;

/**
 * @brief Create a StringView from a C string that will definitly never change or disappear. 
 */
StringView StringView_FromLiteral(const char *str);

/**
 * @brief Return the number of character in the view.
 */
size_t StringView_Length(const StringView *view);

/**
 * @brief Calculate the width of view.
 */
size_t StringView_Width(const StringView *view);

/**
 * @brief Return true if the views are equal.
 */
bool StringView_Equal(const StringView *a, const StringView *b);

/**
 * @brief Return true if the text in view equals cstr. length is the length of cstr in *bytes*
 */
bool StringView_EqualToStr(const StringView *view, const char *cstr, size_t length);

/**
 * @brief Shorten the string to a maximum display width.
 */
StringView StringView_LimitWidth(const StringView *str, int max_width);


/**
 * @brief Iterate over a String or a StringView character by character.
 * 
 * Use it like:
 * StringIterator it = StringIterator_FromString(str);
 * while (StringIterator_Next(&it)) {
 *    // it.current points to the current character
 * }
 */
typedef struct _StringIterator {
    const char *bytes;      //< pointer to a C string taken from a String or a StringView
    size_t bytes_size;      //< length of the string to iterate
    size_t byte_offset;     //< current byte offset during iteration
    size_t char_index;      //< current char index during iteration
    const char *current;    //< pointer to the current character
} StringIterator;

StringIterator StringIterator_FromString(const String *str);
StringIterator StringIterator_FromView(const StringView *view);
bool StringIterator_Next(StringIterator *it);


/********************/
/* String Functions */
/********************/

// Initialization and capacity management (only init/deinit should be needed)
void String_Init(String *string);
void String_Deinit(String *string);

// Creation
String *String_Create();
void String_Destroy(String *string);


/**
 * @brief Return the number of characters in the str.
 */
size_t String_Length(const String *string);

/**
 * @brief Clear the string (set length to 0).
 */
void String_Clear(String *string);

/**
 * @brief Return an empty String.
 */
String String_Empty();

/**
 * @brief Transfer the owenership of src to dest.
 * 
 * Its for situation where dst is already initialized and your want to fill it
 * with the content of another string without moving memory around.
 * dst will be deinitialized!
 */
void String_Take(String *dst, String *src);

/**
 * @brief Create a new String from chstr. (chstr is copied)
 * 
 * @param chstr Pointer to the C string (string is copied)
 * @param length Number of bytes to copy from chstr (strlen(chstr) for complete string).
 * 
 * Note that the length of the resulting String might be less then length (because of multibyte characters).
 */
String String_FromCStr(const char *chstr, size_t length);

/**
 * @brief Create a new String that takes the ownership of chstr. (ownership is transfered)
 * 
 * @param chstr The ownership of this C string will be transfered to the String.
 */
String String_TakeCStr(char *chstr);

/**
 * @brief Return string as C string. The return value is pointer directly into string's internal data and must not be modified or freed.
 */
const char *String_AsCStr(const String *string);

/**
 * @brief Add the first character of ch to str.
 * 
 * If there is no valid UTF8 character it is discarded and str stays unchanged.
 */
void String_AddChar(String *str, const char *ch);

/**
 * @brief Create a copy of a String.
 */
String String_Copy(const String *src);

/**
 * @brief Create String from a format definition (like sprintf())
 */
String String_Format(const char *format, ...);

/**
 * @brief Create a String from a View by copying it's data.
 */
String String_FromView(StringView view);

/**
 * @brief Create a StringView to the complete string.
 */
StringView String_ToView(const String *string);

/**
 * @brief Return the pointer to the char at the given position.
 * 
 * ```
 * String_GetChar(&String_FromCStr("Foobar"), 7) == "oobar"
 * String_GetChar(&String_FromCStr("Foobar"), -2) == "ar"
 * ```
 * 
 * @param string The string to use. This is not const because the internal multibytes map might to be rebuild if it's invalid.
 * @param pos The position of the character (*not* the byte offset). Negative positions are accepted.

 */
const char *String_GetChar(String *string, int pos);

/**
 * @brief Create a (read-only) slice of a String.
 * 
 * Even if the string is not manipulated the internal cache might need an update. This is why string is not const.
 */
StringView String_Slice(String *string, size_t start, size_t end);

/**
 * @brief Create a substring with length characters starting at start. 
 */
String String_Substring(String *string, size_t start, size_t length);

/**
 * @brief Add str2 to the end of str1.
 */
void String_Append(String *str1, const String *str2);

/**
 * @brief Add the text of view to the end str1.
 */
void String_AppendView(String *str, const StringView *view);

/**
 * @brief Repeat str n times and return it.
 */
String String_Repeat(const String *str, size_t n);

/**
 * @brief Create a string consisting of n spaces
 */
String String_Spaces(size_t n);

/**
 * @brief Shorten the string to length characters.
 */
void String_Shorten(String *str, size_t n);


#endif