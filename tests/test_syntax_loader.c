void test_init();
void test_deinit();

#define TEST_INIT { test_init(); }
#define TEST_FINI { test_deinit(); }

#include "acutest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>
#include "io/file.h"
#include "syntax/loader.h"
#include "common/config.h"  // to set a custom project folder

/*****************************************************************/
/* Functions to create a test environment                        */
bool concat_paths(char *dst, const char *path1, const char *path2) {
    if (!dst || !path1 || !path2) return false;
    size_t cap = PATH_MAX;
    size_t l1 = strlen(path1);
    bool need_sep = (l1 > 0 && path1[l1 - 1] != '/');
    const char *p2 = (path2[0] == '/') ? path2 + 1 : path2;
    int n = snprintf(dst, cap, "%s%s%s", path1, need_sep ? "/" : "", p2);
    return n >= 0 && (size_t)n < cap;
}

bool create_syntax_folder() {
    char exe_abs_path[PATH_MAX];
    char data_dir[PATH_MAX];
    char syntax_dir[PATH_MAX];
    
    TEST_CHECK(realpath(Config_GetExePath(), exe_abs_path));
    char *exe_dir = dirname(exe_abs_path);

    if (!concat_paths(data_dir, exe_dir, "data")) {
        return false;
    }
    if (!concat_paths(syntax_dir, data_dir, "syntax")) {
        return false;
    }

    mkdir(data_dir, 0755);
    mkdir(syntax_dir, 0755);
    return true;
}

bool remove_syntax_folder() {
    char exe_abs_path[PATH_MAX];
    
    TEST_CHECK(realpath(Config_GetExePath(), exe_abs_path));
    char *exe_dir = dirname(exe_abs_path);
    char data_dir[PATH_MAX];
    char syntax_dir[PATH_MAX];
    
    if (!concat_paths(data_dir, exe_dir, "data")) {
        return false;
    }
    if (!concat_paths(syntax_dir, data_dir, "syntax")) {
        return false;
    }

    rmdir(syntax_dir);
    rmdir(data_dir);
    return true;
}

void test_init() {
    static char template[] = "/tmp/clieditor_XXXXXX";
    int fd = mkstemp(template);
    TEST_ASSERT(fd != -1);
    Config_Init(template);
    close(fd);

    TEST_ASSERT(create_syntax_folder());
}

void test_deinit() {
    remove_syntax_folder();
    unlink(Config_GetExePath());
    Config_Deinit();
}

void create_testfile(const char *filename, const char *content) {
    char exe_abs_path[PATH_MAX];
    
    TEST_CHECK(realpath(Config_GetExePath(), exe_abs_path));
    char *exe_dir = dirname(exe_abs_path);

    char filepath[PATH_MAX];
    snprintf(filepath, PATH_MAX, "%s/data/syntax/%s", exe_dir, filename);
    FILE *fp = fopen(filepath, "w");
    TEST_ASSERT(fp);
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);
}


void delete_testfile(const char *filename) {
    char exe_abs_path[PATH_MAX];
    
    TEST_CHECK(realpath(Config_GetExePath(), exe_abs_path));
    char *exe_dir = dirname(exe_abs_path);

    char filepath[PATH_MAX];
    snprintf(filepath, PATH_MAX, "%s/data/syntax/%s", exe_dir, filename);
    unlink(filepath);
}

/****************************************************************/
/* Actual testing                                               */

void test_basic(void) {
    const char *ini =
    "[meta]\n"
    "name = TEST\n"
    "\n"
    "[block:root]\n"
    "child_blocks = string\n"
    "\n"
    "[block:string]\n"
    "start = '\n"
    "end = '\n";

    create_testfile("test.ini", ini);
    SyntaxHighlightingLoaderError error;
    SyntaxHighlighting *hl = SyntaxHighlighting_LoadFromFile("test", &error);
    if (!hl) {
        TEST_CHECK(hl);
        TEST_MSG("SyntaxHighlightingLoaderErrorCode: %d", error.code);
        TEST_MSG("message: %s", error.def_error.message.bytes);
        SyntaxHighlightingLoaderError_Deinit(&error);
        return;
    }
    SyntaxHighlighting_Destroy(hl);
    delete_testfile("test.ini");
}

void test_no_filename(void) {
    SyntaxHighlightingLoaderError error;
    SyntaxHighlighting *hl = SyntaxHighlighting_LoadFromFile(NULL, &error);
    TEST_CHECK(!hl);
    TEST_CHECK(error.code == SYNTAX_LOADER_NO_FILENAME);
    SyntaxHighlightingLoaderError_Deinit(&error);
}

void test_file_not_found(void) {
    SyntaxHighlightingLoaderError error;
    SyntaxHighlighting *hl = SyntaxHighlighting_LoadFromFile("test", &error);
    TEST_CHECK(!hl);
    TEST_CHECK(error.code == SYNTAX_LOADER_FILE_NOT_FOUND);
    SyntaxHighlightingLoaderError_Deinit(&error);
}

void test_parse_error(void) {
    const char *ini =
    "[meta\n"    // <---- INI format error
    "name = TEST\n"
    "\n"
    "[block:root]\n"
    "child_blocks = string\n"
    "\n"
    "[block:string]\n"
    "start = '\n"
    "end = '\n";
    create_testfile("test.ini", ini);
    SyntaxHighlightingLoaderError error;
    SyntaxHighlighting *hl = SyntaxHighlighting_LoadFromFile("test", &error);
    TEST_CHECK(!hl);
    TEST_CHECK(error.code == SYNTAX_LOADER_PARSE_ERROR);
    SyntaxHighlightingLoaderError_Deinit(&error);
    delete_testfile("test.ini");
}

void test_definition_error(void) {
    const char *ini =
    "[meta]\n"
    "name = TEST\n"
    "\n"
    "[block:root]\n"
    "child_blocks = string, blub\n"  // <----- non-existing child
    "\n"
    "[block:string]\n"
    "start = '\n"
    "end = '\n";
    create_testfile("test.ini", ini);
    SyntaxHighlightingLoaderError error;
    SyntaxHighlighting *hl = SyntaxHighlighting_LoadFromFile("test", &error);
    TEST_CHECK(!hl);
    TEST_CHECK(error.code == SYNTAX_LOADER_DEFINITION_ERROR);
    SyntaxHighlightingLoaderError_Deinit(&error);
    delete_testfile("test.ini");
}

TEST_LIST = {
    { "SyntaxLoader: Basic", test_basic },
    { "SyntaxLoader: No Filename", test_no_filename },
    { "SyntaxLoader: File Not Found", test_file_not_found },
    { "SyntaxLoader: Parse Error", test_parse_error },
    { "SyntaxLoader: Definition Error", test_definition_error },
    { NULL, NULL }
};
