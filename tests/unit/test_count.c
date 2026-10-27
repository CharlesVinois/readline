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

static void test_whitespace_separates_words(void)
{
    static const unsigned char text[] = "one\ttwo\r\nthree";
    struct readline_counts counts;

    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_OK,
        count_data(text, sizeof text - 1, READLINE_COUNT_WORDS, &counts));
    TEST_ASSERT_EQUAL_UINT64(3, counts.words);
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
    struct readline_counts counts = {1, 2, 3, 4};

    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_INVALID_UTF8,
        count_data(text, sizeof text, READLINE_COUNT_CHARACTERS, &counts));
    TEST_ASSERT_EQUAL_UINT64(1, counts.lines);
    TEST_ASSERT_EQUAL_UINT64(2, counts.words);
    TEST_ASSERT_EQUAL_UINT64(3, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(4, counts.characters);
}

static void test_incomplete_utf8_sequence_is_reported(void)
{
    static const unsigned char text[] = {0xC3};
    struct readline_counts counts;

    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_INCOMPLETE_UTF8,
        count_data(text, sizeof text, READLINE_COUNT_CHARACTERS, &counts));
}

static void test_utf8_boundary_sequences_are_counted(void)
{
    static const unsigned char text[] = {
        0xC2, 0x80,
        0xDF, 0xBF,
        0xE0, 0xA0, 0x80,
        0xE1, 0x80, 0x80,
        0xED, 0x9F, 0xBF,
        0xF0, 0x90, 0x80, 0x80,
        0xF1, 0x80, 0x80, 0x80,
        0xF4, 0x8F, 0xBF, 0xBF
    };
    struct readline_counts counts;

    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_OK,
        count_data(text, sizeof text, READLINE_COUNT_CHARACTERS, &counts));
    TEST_ASSERT_EQUAL_UINT64(8, counts.characters);
}

static void test_utf8_rejects_out_of_range_continuation_bytes(void)
{
    static const unsigned char invalid_sequences[][2] = {
        {0xC2, 0x7F},
        {0xC2, 0xC0},
        {0xE0, 0x9F},
        {0xED, 0xA0},
        {0xF0, 0x8F},
        {0xF4, 0x90}
    };
    struct readline_counts counts;

    for (size_t i = 0; i < sizeof invalid_sequences / sizeof invalid_sequences[0];
         i++) {
        TEST_ASSERT_EQUAL_INT(
            READLINE_STATUS_INVALID_UTF8,
            count_data(invalid_sequences[i], sizeof invalid_sequences[i],
                       READLINE_COUNT_CHARACTERS, &counts));
    }
}

static void test_utf8_rejects_invalid_leading_bytes(void)
{
    static const unsigned char invalid_bytes[] = {
        0x80, 0xC0, 0xC1, 0xF5, 0xF6, 0xF7, 0xFF
    };
    struct readline_counts counts;

    for (size_t i = 0; i < sizeof invalid_bytes; i++) {
        TEST_ASSERT_EQUAL_INT(
            READLINE_STATUS_INVALID_UTF8,
            count_data(&invalid_bytes[i], 1, READLINE_COUNT_CHARACTERS,
                       &counts));
    }
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

static void test_reports_read_error_from_a_directory_stream(void)
{
    FILE *directory = fopen(".", "rb");
    struct readline_counts counts = {1, 2, 3, 4};

    TEST_ASSERT_NOT_NULL(directory);
    TEST_ASSERT_EQUAL_INT(
        READLINE_STATUS_READ_ERROR,
        readline_count_stream(directory, READLINE_COUNT_BYTES, &counts));
    TEST_ASSERT_EQUAL_UINT64(1, counts.lines);
    TEST_ASSERT_EQUAL_UINT64(2, counts.words);
    TEST_ASSERT_EQUAL_UINT64(3, counts.bytes);
    TEST_ASSERT_EQUAL_UINT64(4, counts.characters);
    fclose(directory);
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
    RUN_TEST(test_whitespace_separates_words);
    RUN_TEST(test_utf8_characters_can_use_more_than_one_byte);
    RUN_TEST(test_invalid_utf8_is_reported_when_requested);
    RUN_TEST(test_incomplete_utf8_sequence_is_reported);
    RUN_TEST(test_utf8_boundary_sequences_are_counted);
    RUN_TEST(test_utf8_rejects_out_of_range_continuation_bytes);
    RUN_TEST(test_utf8_rejects_invalid_leading_bytes);
    RUN_TEST(test_utf8_is_not_validated_when_not_counting_characters);
    RUN_TEST(test_rejects_invalid_arguments);
    RUN_TEST(test_keeps_the_stream_open_for_the_caller);
    RUN_TEST(test_reports_read_error_from_a_directory_stream);
    return UNITY_END();
}
