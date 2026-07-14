#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdUserStringParse.h"

extern void *cmd_MemoryAlloc(size_t bytes);
extern void cmd_MemoryFree(void *mem);
void* cmd_MemoryAlloc(size_t bytes) { return malloc(bytes); }
void cmd_MemoryFree(void* mem) { free(mem); }

static int testCnt = 0;
static int failCnt = 0;

static void test_parse(const char *input, int expectedCnt, const char **expectedTokens)
{
    testCnt++;
    printf("\n[TEST %d] input: \"%s\"\n", testCnt, input);

    void *ret = ParseSpace(input);
    int cnt = userParse_GetUserParamCnt();
    userString *tokens = userParse_pUserData();

    int pass = 1;
    if (cnt != expectedCnt) {
        printf("  FAIL: expected %d tokens, got %d\n", expectedCnt, cnt);
        pass = 0;
    }

    for (int i = 0; i < cnt && i < expectedCnt; i++) {
        int len = (int)tokens[i].len;
        const char *expected = expectedTokens[i];
        int expectedLen = (int)strlen(expected);

        if (len != expectedLen || memcmp(tokens[i].strHead, expected, len) != 0) {
            printf("  FAIL token[%d]: expected \"%s\" (len=%d), got \"%.*s\" (len=%d)\n",
                   i, expected, expectedLen, len, (char*)tokens[i].strHead, len);
            pass = 0;
        } else {
            printf("  OK   token[%d]: \"%.*s\" (len=%d)\n", i, len, (char*)tokens[i].strHead, len);
        }
    }
    for (int i = cnt; i < expectedCnt; i++) {
        printf("  FAIL token[%d]: expected \"%s\" but missing\n", i, expectedTokens[i]);
        pass = 0;
    }
    for (int i = expectedCnt; i < cnt; i++) {
        printf("  FAIL token[%d]: unexpected extra token \"%.*s\"\n",
               i, (int)tokens[i].len, (char*)tokens[i].strHead);
        pass = 0;
    }

    if (pass) printf("  => PASS\n");
    else failCnt++;

    RESET_USERDATA_RECORD();
}

#if ENABLE_WCHAR
#include <wchar.h>
static void test_parse_w(const wchar_t *input, int expectedCnt, const char **expectedTokens)
{
    testCnt++;
    printf("\n[TEST %d] input: \"%ls\"\n", testCnt, input);

    void *ret = ParseSpaceW(input);
    int cnt = userParse_GetUserParamCnt();
    userString *tokens = userParse_pUserData();

    int pass = 1;
    if (cnt != expectedCnt) {
        printf("  FAIL: expected %d tokens, got %d\n", expectedCnt, cnt);
        pass = 0;
    }

    for (int i = 0; i < cnt && i < expectedCnt; i++) {
        int len = (int)tokens[i].len;
        const char *expected = expectedTokens[i];
        int expectedLen = (int)strlen(expected);

        /* narrow expected -> wide temp buffer, then wmemcmp */
        wchar_t wbuf[128];
        for (int j = 0; j < expectedLen; j++) wbuf[j] = (wchar_t)(unsigned char)expected[j];
        wbuf[expectedLen] = L'\0';
        int match = (len == expectedLen && wmemcmp(tokens[i].strHead, wbuf, len) == 0);

        if (!match) {
            printf("  FAIL token[%d]: expected \"%s\" (len=%d), got \"%.*ls\" (len=%d)\n",
                   i, expected, expectedLen, len, (wchar_t*)tokens[i].strHead, len);
            pass = 0;
        } else {
            printf("  OK   token[%d]: \"%.*ls\" (len=%d)\n", i, len, (wchar_t*)tokens[i].strHead, len);
        }
    }
    for (int i = cnt; i < expectedCnt; i++) {
        printf("  FAIL token[%d]: expected \"%s\" but missing\n", i, expectedTokens[i]);
        pass = 0;
    }
    for (int i = expectedCnt; i < cnt; i++) {
        printf("  FAIL token[%d]: unexpected extra token \"%.*ls\"\n",
               i, (int)tokens[i].len, (wchar_t*)tokens[i].strHead);
        pass = 0;
    }

    if (pass) printf("  => PASS\n");
    else failCnt++;

    RESET_USERDATA_RECORD();
}
#endif

int main(void)
{
    printf("=== Tokenizer Double-Quote Test Suite ===\n");

    /* 1. basic space split */
    {
        const char *expected[] = {"cmd", "param"};
        test_parse("cmd param", 2, expected);
    }

    /* 2. single quoted token */
    {
        const char *expected[] = {"cmd", "hello world"};
        test_parse("cmd \"hello world\"", 2, expected);
    }

    /* 3. quoted at beginning */
    {
        const char *expected[] = {"raw data", "cmd", "param"};
        test_parse("\"raw data\" cmd param", 3, expected);
    }

    /* 4. multiple quoted tokens */
    {
        const char *expected[] = {"cmd", "a b", "c d"};
        test_parse("cmd \"a b\" \"c d\"", 3, expected);
    }

    /* 5. unclosed quote */
    {
        const char *expected[] = {"cmd", "unclosed string"};
        test_parse("cmd \"unclosed string", 2, expected);
    }

    /* 6. empty quote */
    {
        const char *expected[] = {"cmd", ""};
        test_parse("cmd \"\"", 2, expected);
    }

    /* 7. path with spaces */
    {
        const char *expected[] = {"fatfs", "write", "/path/with space/file.bin"};
        test_parse("fatfs write \"/path/with space/file.bin\"", 3, expected);
    }

    /* 8. Chinese characters in quotes */
    {
        const char *expected[] = {"wait", "dat", "names", "\xe5\xbc\xa0\xe4\xb8\x89"};
        test_parse("wait dat names \"\xe5\xbc\xa0\xe4\xb8\x89\"", 4, expected);
    }

    /* 9. no quotes, normal multi-space */
    {
        const char *expected[] = {"a", "b", "c"};
        test_parse("  a   b  c  ", 3, expected);
    }

    /* 10. single word */
    {
        const char *expected[] = {"hello"};
        test_parse("hello", 1, expected);
    }

    /* 11. quote then normal text adjacent */
    {
        const char *expected[] = {"hello", "world"};
        test_parse("\"hello\"world", 2, expected);
    }

    /* 12. normal then quote adjacent */
    {
        const char *expected[] = {"hello", "world"};
        test_parse("hello\"world\"", 2, expected);
    }

    /* 13. only quoted */
    {
        const char *expected[] = {"only quoted"};
        test_parse("\"only quoted\"", 1, expected);
    }

    printf("\n=== Narrow-char Results: %d/%d passed ===\n", testCnt - failCnt, testCnt);

#if ENABLE_WCHAR
    /* ========== Wide-char tests ========== */
    printf("\n--- Wide-char tests ---\n");

    /* W1. basic wide char space split */
    {
        const char *expected[] = {"cmd", "param"};
        test_parse_w(L"cmd param", 2, expected);
    }

    /* W2. wide char quoted */
    {
        const char *expected[] = {"cmd", "hello world"};
        test_parse_w(L"cmd \"hello world\"", 2, expected);
    }

    /* W3. wide char unclosed quote */
    {
        const char *expected[] = {"cmd", "unclosed"};
        test_parse_w(L"cmd \"unclosed", 2, expected);
    }

    /* W4. wide char empty quote */
    {
        const char *expected[] = {"cmd", ""};
        test_parse_w(L"cmd \"\"", 2, expected);
    }

    /* W5. wide char multiple spaces */
    {
        const char *expected[] = {"a", "b", "c"};
        test_parse_w(L"  a   b  c  ", 3, expected);
    }
#endif

    printf("\n=== Total Results: %d/%d passed ===\n", testCnt - failCnt, testCnt);
    return failCnt ? 1 : 0;
}
