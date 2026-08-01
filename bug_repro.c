/*
 * Minimal reproducer for the trailing decimal-point bug in parson <= 1.5.3.
 *
 * RFC 8259 ┬º6 grammar:
 *   number = [ minus ] int [ frac ] [ exp ]
 *   frac   = decimal-point 1*DIGIT
 *
 * strtod(3) in C accepts "1." successfully, returning 1.0.
 * parson's is_decimal() only checked for hex notation (x/X), so it also
 * accepted "1." without error.  This means any caller using json_parse_string()
 * as a validation gate would pass inputs that every other RFC-conformant parser
 * (Python json, Go encoding/json, jq, etc.) correctly rejects.
 *
 * Build: gcc -o bug_repro bug_repro.c parson.c -lm
 * Run:   ./bug_repro
 */

#include "parson.h"
#include <stdio.h>
#include <stdlib.h>

static int check(const char *label, const char *input, int expect_success) {
    JSON_Value *v = json_parse_string(input);
    int got_success = (v != NULL);
    json_value_free(v);
    if (got_success == expect_success) {
        printf("[PASS] %-40s  (got %s)\n", label, got_success ? "success" : "failure");
        return 1;
    }
    printf("[FAIL] %-40s  expected %s, got %s\n",
           label,
           expect_success ? "success" : "failure",
           got_success    ? "success" : "failure");
    return 0;
}

int main(void) {
    int passes = 0, total = 0;

    /* These are the inputs that were silently accepted before the fix
       but are invalid under RFC 8259 ┬º6 because they have no digit
       after the decimal point. */
    puts("=== Cases that must be REJECTED (invalid per RFC 8259 ┬º6) ===");
    passes += check("{\"n\": 1.}",         "{\"n\": 1.}",         0); total++;
    passes += check("{\"n\": -0.}",        "{\"n\": -0.}",        0); total++;
    passes += check("{\"n\": 1.e5}",       "{\"n\": 1.e5}",       0); total++;
    passes += check("{\"n\": 123.}",       "{\"n\": 123.}",       0); total++;

    puts("\n=== Cases that must be ACCEPTED (valid per RFC 8259 ┬º6) ===");
    passes += check("{\"n\": 1.0}",        "{\"n\": 1.0}",        1); total++;
    passes += check("{\"n\": -0.5}",       "{\"n\": -0.5}",       1); total++;
    passes += check("{\"n\": 1.5e10}",     "{\"n\": 1.5e10}",     1); total++;
    passes += check("{\"n\": 0}",          "{\"n\": 0}",          1); total++;
    passes += check("{\"n\": -1}",         "{\"n\": -1}",         1); total++;
    passes += check("{\"n\": 1e5}",        "{\"n\": 1e5}",        1); total++;

    printf("\n%d/%d tests passed\n", passes, total);
    return (passes == total) ? 0 : 1;
}
