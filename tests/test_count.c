#include "count.h"

#include "unity.h"

static enum CountStatus count_data(const unsigned char *data, size_t size,
                                   int count_characters, struct Counts *counts)
{
    FILE *file = tmpfile();
    if (file == NULL) {
        return COUNT_STATUS_READ_ERROR;
    }

    if (size > 0 && fwrite(data, 1, size, file) != size) {
        fclose(file);
        return COUNT_STATUS_READ_ERROR;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return COUNT_STATUS_READ_ERROR;
    }

    enum CountStatus status = count_stream(file, count_characters, counts);
    if (fclose(file) == EOF && status == COUNT_STATUS_OK) {
        return COUNT_STATUS_READ_ERROR;
    }
    return status;
}

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_empty_stream_has_zero_counts(void)
{
    struct Counts counts;
    TEST_ASSERT_EQUAL_INT(COUNT_STATUS_OK, count_data(NULL, 0, 1, &counts));
    TEST_ASSERT_EQUAL_UINT64(0, counts.lines);
    TEST_ASSERT_EQUAL_UINT64(0, counts.words);
    TEST_ASSERT_EQUAL_UINT64(0, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(0, counts.characters);
}

static void test_counts_lines_words_bytes_and_ascii_characters(void)
{
    static const unsigned char text[] = "one two\nthree\n";
    struct Counts counts;

    TEST_ASSERT_EQUAL_INT(
        COUNT_STATUS_OK, count_data(text, sizeof text - 1, 1, &counts));
    TEST_ASSERT_EQUAL_UINT64(2, counts.lines);
    TEST_ASSERT_EQUAL_UINT64(3, counts.words);
    TEST_ASSERT_EQUAL_UINT64(14, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(14, counts.characters);
}

static void test_counts_final_line_without_newline(void)
{
    static const unsigned char text[] = "one\ntwo";
    struct Counts counts;

    TEST_ASSERT_EQUAL_INT(
        COUNT_STATUS_OK, count_data(text, sizeof text - 1, 1, &counts));
    TEST_ASSERT_EQUAL_UINT64(2, counts.lines);
}

static void test_utf8_characters_can_use_more_than_one_byte(void)
{
    static const unsigned char text[] = "été\n";
    struct Counts counts;

    TEST_ASSERT_EQUAL_INT(
        COUNT_STATUS_OK, count_data(text, sizeof text - 1, 1, &counts));
    TEST_ASSERT_EQUAL_UINT64(6, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(4, counts.characters);
}

static void test_invalid_utf8_is_reported_when_requested(void)
{
    static const unsigned char text[] = {0xFF};
    struct Counts counts;

    TEST_ASSERT_EQUAL_INT(COUNT_STATUS_INVALID_UTF8,
                          count_data(text, sizeof text, 1, &counts));
}

static void test_incomplete_utf8_sequence_is_reported(void)
{
    static const unsigned char text[] = {0xC3};
    struct Counts counts;

    TEST_ASSERT_EQUAL_INT(COUNT_STATUS_INCOMPLETE_UTF8,
                          count_data(text, sizeof text, 1, &counts));
}

static void test_utf8_is_not_validated_when_not_counting_characters(void)
{
    static const unsigned char text[] = {0xFF};
    struct Counts counts;

    TEST_ASSERT_EQUAL_INT(COUNT_STATUS_OK,
                          count_data(text, sizeof text, 0, &counts));
    TEST_ASSERT_EQUAL_UINT64(1, counts.bytes);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_empty_stream_has_zero_counts);
    RUN_TEST(test_counts_lines_words_bytes_and_ascii_characters);
    RUN_TEST(test_counts_final_line_without_newline);
    RUN_TEST(test_utf8_characters_can_use_more_than_one_byte);
    RUN_TEST(test_invalid_utf8_is_reported_when_requested);
    RUN_TEST(test_incomplete_utf8_sequence_is_reported);
    RUN_TEST(test_utf8_is_not_validated_when_not_counting_characters);
    return UNITY_END();
}
