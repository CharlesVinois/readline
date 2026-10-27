#include "readline_stats.h"

#include "unity.h"

static enum readline_status count_data(const unsigned char *data, size_t size,
                                       unsigned int options,
                                       struct readline_counts *counts)
{
    FILE *file = tmpfile();
    if (file == NULL) {
        return READLINE_STATUS_READ_ERROR;
    }

    if (size > 0 && fwrite(data, 1, size, file) != size) {
        fclose(file);
        return READLINE_STATUS_READ_ERROR;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return READLINE_STATUS_READ_ERROR;
    }

    enum readline_status status = readline_count_stream(file, options, counts);
    if (fclose(file) == EOF && status == READLINE_STATUS_OK) {
        return READLINE_STATUS_READ_ERROR;
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
    struct readline_counts counts;
    TEST_ASSERT_EQUAL_INT(READLINE_STATUS_OK,
                          count_data(NULL, 0, READLINE_COUNT_ALL, &counts));
    TEST_ASSERT_EQUAL_UINT64(0, counts.lines);
    TEST_ASSERT_EQUAL_UINT64(0, counts.words);
    TEST_ASSERT_EQUAL_UINT64(0, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(0, counts.characters);
}

static void test_counts_lines_words_bytes_and_ascii_characters(void)
{
    static const unsigned char text[] = "one two\nthree\n";
    struct readline_counts counts;

    TEST_ASSERT_EQUAL_INT(READLINE_STATUS_OK,
                          count_data(text, sizeof text - 1, READLINE_COUNT_ALL,
                                     &counts));
    TEST_ASSERT_EQUAL_UINT64(2, counts.lines);
    TEST_ASSERT_EQUAL_UINT64(3, counts.words);
    TEST_ASSERT_EQUAL_UINT64(14, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(14, counts.characters);
}

static void test_counts_final_line_without_newline(void)
{
    static const unsigned char text[] = "one\ntwo";
    struct readline_counts counts;

    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_OK,
        count_data(text, sizeof text - 1, READLINE_COUNT_LINES, &counts));
    TEST_ASSERT_EQUAL_UINT64(2, counts.lines);
    TEST_ASSERT_EQUAL_UINT64(0, counts.words);
    TEST_ASSERT_EQUAL_UINT64(0, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(0, counts.characters);
}

static void test_utf8_characters_can_use_more_than_one_byte(void)
{
    static const unsigned char text[] = "été\n";
    struct readline_counts counts;

    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_OK,
        count_data(text, sizeof text - 1,
                   READLINE_COUNT_BYTES | READLINE_COUNT_CHARACTERS, &counts));
    TEST_ASSERT_EQUAL_UINT64(6, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(4, counts.characters);
}

static void test_invalid_utf8_is_reported_when_requested(void)
{
    static const unsigned char text[] = {0xFF};
    struct readline_counts counts;

    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_INVALID_UTF8,
        count_data(text, sizeof text, READLINE_COUNT_CHARACTERS, &counts));
}

static void test_incomplete_utf8_sequence_is_reported(void)
{
    static const unsigned char text[] = {0xC3};
    struct readline_counts counts;

    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_INCOMPLETE_UTF8,
        count_data(text, sizeof text, READLINE_COUNT_CHARACTERS, &counts));
}

static void test_utf8_is_not_validated_when_not_counting_characters(void)
{
    static const unsigned char text[] = {0xFF};
    struct readline_counts counts;

    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_OK,
        count_data(text, sizeof text, READLINE_COUNT_BYTES, &counts));
    TEST_ASSERT_EQUAL_UINT64(1, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(0, counts.characters);
}

static void test_rejects_invalid_arguments(void)
{
    static const unsigned char text[] = "data";
    FILE *file = tmpfile();
    struct readline_counts counts = {1, 2, 3, 4};

    TEST_ASSERT_NOT_NULL(file);
    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_INVALID_ARGUMENT,
        readline_count_stream(file, READLINE_COUNT_LINES, NULL));
    TEST_ASSERT_EQUAL_INT(READLINE_STATUS_INVALID_ARGUMENT,
                          readline_count_stream(NULL, READLINE_COUNT_LINES,
                                                &counts));
    TEST_ASSERT_EQUAL_INT(READLINE_STATUS_INVALID_ARGUMENT,
                          count_data(text, sizeof text - 1, 0, &counts));
    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_INVALID_ARGUMENT,
        count_data(text, sizeof text - 1, READLINE_COUNT_ALL | (1u << 7),
                   &counts));
    TEST_ASSERT_EQUAL_UINT64(1, counts.lines);
    TEST_ASSERT_EQUAL_UINT64(2, counts.words);
    TEST_ASSERT_EQUAL_UINT64(3, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(4, counts.characters);
    fclose(file);
}

static void test_keeps_the_stream_open_for_the_caller(void)
{
    FILE *file = tmpfile();
    struct readline_counts counts;

    TEST_ASSERT_NOT_NULL(file);
    TEST_ASSERT_TRUE(fputs("data", file) >= 0);
    TEST_ASSERT_EQUAL_INT(0, fseek(file, 0, SEEK_SET));
    TEST_ASSERT_EQUAL_INT(READLINE_STATUS_OK,
                          readline_count_stream(file, READLINE_COUNT_BYTES,
                                                &counts));
    TEST_ASSERT_EQUAL_UINT64(4, counts.bytes);
    TEST_ASSERT_EQUAL_INT(0, fseek(file, 0, SEEK_SET));
    TEST_ASSERT_EQUAL_INT('d', fgetc(file));
    fclose(file);
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
    RUN_TEST(test_rejects_invalid_arguments);
    RUN_TEST(test_keeps_the_stream_open_for_the_caller);
    return UNITY_END();
}
