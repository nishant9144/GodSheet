#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Declarations/ds.h"
#include "Declarations/backend.h"
#include "Declarations/parser.h"
#include "Declarations/frontend.h"

// Basic assertion macro
#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s\n", message); \
            return 0; \
        } \
    } while (0)

// Equality assertion macro
#define ASSERT_EQ(actual, expected, message) \
    do { \
        if ((actual) != (expected)) { \
            printf("FAIL: %s\nExpected: %d\nGot: %d\n", \
                   message, expected, actual); \
            return 0; \
        } \
    } while (0)

// Status assertion macro
#define ASSERT_STATUS(sheet, expected, message) \
    do { \
        if ((sheet)->last_status != (expected)) { \
            printf("FAIL: %s\nExpected status: %d\nGot status: %d\n", \
                   message, expected, (sheet)->last_status); \
            return 0; \
        } \
    } while (0)

// Debug print function
void debug_print_cell(Cell* cell) {
    if (!cell) {
        printf("Cell is NULL\n");
        return;
    }
    printf("Cell [%d,%d]: value=%d, type=%c, has_error=%d\n",
           cell->row, cell->col, cell->value, cell->type, cell->has_error);
    if (cell->dependencies.first.i != -1 || cell->dependencies.first.j != -1 ||
        cell->dependencies.second.i != -1 || cell->dependencies.second.j != -1) {
        printf("  Has dependencies\n");
    }
    if (cell->dependents) {
        printf("  Has dependents\n");
    }
}



Spreadsheet* setup_with_size(int rows, int cols) {
    printf("Creating %dx%d spreadsheet...\n", rows, cols);
    Spreadsheet* sheet = create_spreadsheet(rows, cols);
    if (sheet == NULL) {
        fprintf(stderr, "Failed to create spreadsheet\n");
        return NULL;
    }
    
    // Verify the sheet structure
    if (sheet->cells == NULL) {
        fprintf(stderr, "Sheet cells array is NULL\n");
        free(sheet);
        return NULL;
    }
    
    for (int i = 0; i < sheet->totalRows; i++) {
        if (sheet->cells[i] == NULL) {
            fprintf(stderr, "Row %d is NULL\n", i);
            free_spreadsheet(sheet);
            return NULL;
        }
    }
    
    printf("Spreadsheet created successfully\n");
    return sheet;
}


Spreadsheet* setup() {
    printf("Creating spreadsheet...\n");
    Spreadsheet* sheet = create_spreadsheet(10, 10);
    if (sheet == NULL) {
        fprintf(stderr, "Failed to create spreadsheet\n");
        return NULL;
    }
    
    // Verify the sheet structure
    if (sheet->cells == NULL) {
        fprintf(stderr, "Sheet cells array is NULL\n");
        free(sheet);
        return NULL;
    }
    
    for (int i = 0; i < sheet->totalRows; i++) {
        if (sheet->cells[i] == NULL) {
            fprintf(stderr, "Row %d is NULL\n", i);
            free_spreadsheet(sheet);
            return NULL;
        }
    }
    
    printf("Spreadsheet created successfully\n");
    return sheet;
}

void teardown(Spreadsheet* sheet) {
    if (sheet != NULL) {
        printf("Freeing spreadsheet...\n");
        free_spreadsheet(sheet);
        printf("Spreadsheet freed\n");
    }
}


// Basic Cell Operations
int test_basic_cell_operations() {
    struct TestCase {
        const char* command;
        int expected_value;
        const char* description;
    } tests[] = {
        {"A1=42", 42, "Simple assignment"},
        {"B1=-10", -10, "Negative number"},
        {"C1=0", 0, "Zero assignment"}
    };
    
    Spreadsheet* sheet = setup();
    if (!sheet) return 0;

    const size_t num_tests = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        char cmd[256];
        strncpy(cmd, tests[i].command, sizeof(cmd) - 1);
        cmd[sizeof(cmd) - 1] = '\0';
        
        process_command(sheet, cmd);
        ASSERT(sheet->cells[0][i].value == tests[i].expected_value,
               tests[i].description);
    }

    teardown(sheet);
    return 1;
}

// Arithmetic Operations
int test_arithmetic_operations() {
    struct TestCase {
        const char* setup;
        const char* command;
        int expected_value;
        const char* description;
    } tests[] = {
        {"A1=5", "B1=A1+3", 8, "Addition"},
        {"A1=10", "B1=A1-4", 6, "Subtraction"},
        {"A1=6", "B1=A1*7", 42, "Multiplication"},
        {"A1=15", "B1=A1/3", 5, "Division"}
    };

    Spreadsheet* sheet = setup();
    if (!sheet) return 0;

    const size_t num_tests = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        char setup_cmd[256], test_cmd[256];
        strncpy(setup_cmd, tests[i].setup, sizeof(setup_cmd) - 1);
        strncpy(test_cmd, tests[i].command, sizeof(test_cmd) - 1);
        setup_cmd[sizeof(setup_cmd) - 1] = '\0';
        test_cmd[sizeof(test_cmd) - 1] = '\0';
        
        process_command(sheet, setup_cmd);
        process_command(sheet, test_cmd);
        ASSERT(sheet->cells[0][1].value == tests[i].expected_value,
               tests[i].description);
    }

    teardown(sheet);
    return 1;
}

// Function Tests
int test_spreadsheet_functions() {
    struct TestCase {
        const char* setup[3];
        const char* command;
        int expected_value;
        const char* description;
    } tests[] = {
        {{"A1=10", "A2=20", "A3=30"}, "B1=SUM(A1:A3)", 60, "SUM function"},
        {{"A1=10", "A2=20", "A3=30"}, "B1=AVG(A1:A3)", 20, "AVG function"},
        {{"A1=10", "A2=20", "A3=30"}, "B1=MAX(A1:A3)", 30, "MAX function"},
        {{"A1=10", "A2=20", "A3=30"}, "B1=MIN(A1:A3)", 10, "MIN function"}
    };

    Spreadsheet* sheet = setup();
    if (!sheet) return 0;

    const size_t num_tests = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        char cmd[256];
        // Setup cells
        for (size_t j = 0; j < 3; j++) {
            strncpy(cmd, tests[i].setup[j], sizeof(cmd) - 1);
            cmd[sizeof(cmd) - 1] = '\0';
            process_command(sheet, cmd);
        }
        // Test function
        strncpy(cmd, tests[i].command, sizeof(cmd) - 1);
        cmd[sizeof(cmd) - 1] = '\0';
        process_command(sheet, cmd);
        ASSERT(sheet->cells[0][1].value == tests[i].expected_value,
               tests[i].description);
    }

    teardown(sheet);
    return 1;
}



int test_division_by_zero() {
    printf("Starting test_division_by_zero...\n");
    
    Spreadsheet *sheet = setup();
    if (sheet == NULL) return 0;

    // Create command strings
    char cmd1[] = "A1=10";
    char cmd2[] = "A2=0";
    char cmd3[] = "A3=A1/A2";

    printf("Processing %s...\n", cmd1);
    process_command(sheet, cmd1);
    if (sheet->last_status != STATUS_OK) {
        printf("FAIL: Failed to set A1=10\n");
        teardown(sheet);
        return 0;
    }

    printf("Processing %s...\n", cmd2);
    process_command(sheet, cmd2);
    if (sheet->last_status != STATUS_OK) {
        printf("FAIL: Failed to set A2=0\n");
        teardown(sheet);
        return 0;
    }

    printf("Processing %s...\n", cmd3);
    process_command(sheet, cmd3);

    // Verify cell access
    if (sheet->cells == NULL || sheet->cells[2] == NULL) {
        printf("FAIL: Invalid cell access\n");
        teardown(sheet);
        return 0;
    }

    ASSERT(sheet->cells[2][0].has_error, "Division by zero should set error");
    teardown(sheet);
    return 1;
}

int test_circular_dependencies() {
    printf("Starting test_circular_dependencies...\n");
    
    Spreadsheet* sheet = setup();
    if (sheet == NULL) return 0;

    // Debug print initial state
    printf("Initial state of A1:\n");
    debug_print_cell(&sheet->cells[0][0]);
    printf("Initial state of A2:\n");
    debug_print_cell(&sheet->cells[0][1]);

    // First command
    printf("\nProcessing A1=A2+1...\n");
    char cmd1[] = "A1=A2+1";
    process_command(sheet, cmd1);
    
    printf("After first command:\n");
    printf("Status: %d\n", sheet->last_status);
    debug_print_cell(&sheet->cells[0][0]);
    debug_print_cell(&sheet->cells[0][1]);

    if (sheet->last_status != STATUS_OK) {
        printf("FAIL: Failed to set A1=A2+1\n");
        teardown(sheet);
        return 0;
    }

    // Second command
    printf("\nProcessing A2=A1+1...\n");
    char cmd2[] = "A2=A1+1";
    process_command(sheet, cmd2);

    printf("After second command:\n");
    printf("Status: %d\n", sheet->last_status);
    debug_print_cell(&sheet->cells[0][0]);
    debug_print_cell(&sheet->cells[0][1]);

    ASSERT(sheet->last_status == ERR_CIRCULAR_REFERENCE, "Circular dependency not detected");
    
    printf("Test completed successfully\n");
    teardown(sheet);
    return 1;
}


int test_formula_evaluation() {
    printf("Starting test_formula_evaluation...\n");
    
    Spreadsheet* sheet = setup();
    if (sheet == NULL) return 0;

    printf("Initial state of A1, A2, A3:\n");
    debug_print_cell(&sheet->cells[0][0]);
    debug_print_cell(&sheet->cells[1][0]);
    debug_print_cell(&sheet->cells[2][0]);

    char cmd1[] = "A1=5";
    char cmd2[] = "A2=10";
    char cmd3[] = "A3=A1+A2";

    printf("\nProcessing %s...\n", cmd1);
    process_command(sheet, cmd1);
    debug_print_cell(&sheet->cells[0][0]);

    printf("\nProcessing %s...\n", cmd2);
    process_command(sheet, cmd2);
    debug_print_cell(&sheet->cells[1][0]);

    printf("\nProcessing %s...\n", cmd3);
    process_command(sheet, cmd3);
    debug_print_cell(&sheet->cells[2][0]);

    ASSERT(sheet->cells[2][0].value == 15, "Formula evaluation failed (5 + 10 = 15)");
    
    printf("Test completed successfully\n");
    teardown(sheet);
    return 1;
}

int test_scroll_commands() {
    printf("Starting test_scroll_commands...\n");
    
    Spreadsheet* sheet = setup();
    if (sheet == NULL) return 0;

    printf("Initial state:\n");
    printf("scroll_row=%d, totalRows=%d, VIEWPORT_ROWS=%d\n",
           sheet->scroll_row, sheet->totalRows, VIEWPORT_ROWS);

    // Test scroll down
    printf("\nTesting scroll down...\n");
    handle_scroll(sheet, 's');
    printf("After scroll: scroll_row=%d\n", sheet->scroll_row);
    
    //it should work as folowed by logic below  Hopefully it works as expected
    // For a 10x10 spreadsheet with VIEWPORT_ROWS=10,
    // scrolling down shouldn't change position because:
    // totalRows(10) - VIEWPORT_ROWS(10) = 0
    // Therefore scroll_row should remain 0
    ASSERT(sheet->scroll_row == 0,
           "Scroll down should not change position for a 10x10 sheet");

    // larger spreadsheet to test scrolling
    teardown(sheet);
    sheet = create_spreadsheet(30, 30);
    if (sheet == NULL) return 0;

    printf("\nTesting with larger spreadsheet (30x30):\n");
    printf("Initial scroll_row=%d\n", sheet->scroll_row);

    // Test scroll down
    handle_scroll(sheet, 's');
    printf("After scroll down: scroll_row=%d\n", sheet->scroll_row);
    ASSERT(sheet->scroll_row == 10,
           "Scroll down should increment scroll_row by 10 in larger sheet");

    // Test scroll up
    handle_scroll(sheet, 'w');
    printf("After scroll up: scroll_row=%d\n", sheet->scroll_row);
    ASSERT(sheet->scroll_row == 0,
           "Scroll up should decrement scroll_row by 10");

    printf("Test completed successfully\n");
    teardown(sheet);
    return 1;
}


int test_advanced_operations() {
    printf("Starting test_advanced_operations...\n");
    
    Spreadsheet* sheet = setup_with_size(999, 1000);
    if (sheet == NULL) return 0;

    // Basic value assignments (same as before)
    const char* basic_cmds[] = {
        "A1=10", "B1=20", "C1=30", "D1=40", "E1=50",
        "F1=60", "G1=70", "H1=80", "I1=90", "J1=100"
    };

    printf("\nTesting basic assignments...\n");
    for (int i = 0; i < 10; i++) {
        char cmd[256];
        strncpy(cmd, basic_cmds[i], sizeof(cmd) - 1);
        cmd[sizeof(cmd) - 1] = '\0';
        
        printf("Processing %s...\n", cmd);
        process_command(sheet, cmd);
        if (sheet->last_status != STATUS_OK) {
            printf("FAIL: Basic assignment failed with status %d\n", sheet->last_status);
            teardown(sheet);
            return 0;
        }
        debug_print_cell(&sheet->cells[0][i]);
    }

    // Arithmetic operations (same as before)
    const char* arithmetic_cmds[] = {
        "K1=A1+B1",   // 30
        "L1=C1-D1",   // -10
        "M1=E1*F1",   // 3000
        "N1=G1/H1"    // Should be 0 with integer division
    };

    printf("\nTesting arithmetic operations...\n");
    for (int i = 0; i < 4; i++) {
        char cmd[256];
        strncpy(cmd, arithmetic_cmds[i], sizeof(cmd) - 1);
        cmd[sizeof(cmd) - 1] = '\0';
        
        printf("Processing %s...\n", cmd);
        process_command(sheet, cmd);
        if (sheet->last_status != STATUS_OK) {
            printf("FAIL: Arithmetic operation failed with status %d\n", sheet->last_status);
            teardown(sheet);
            return 0;
        }
        debug_print_cell(&sheet->cells[0][10+i]);
    }

    // Test SUM function separately first
    printf("\nTesting SUM function...\n");
    char sum_cmd[] = "O1=SUM(A1:J1)";
    printf("Processing %s...\n", sum_cmd);
    process_command(sheet, sum_cmd);
    if (sheet->last_status != STATUS_OK) {
        printf("FAIL: SUM function failed with status %d\n", sheet->last_status);
        teardown(sheet);
        return 0;
    }
    printf("SUM result: ");
    debug_print_cell(&sheet->cells[0][14]);

    // Test other functions one by one
    printf("\nTesting AVG function...\n");
    char avg_cmd[] = "P1=AVG(A1:J1)";
    printf("Processing %s...\n", avg_cmd);
    process_command(sheet, avg_cmd);
    if (sheet->last_status != STATUS_OK) {
        printf("FAIL: AVG function failed with status %d\n", sheet->last_status);
        teardown(sheet);
        return 0;
    }
    printf("AVG result: ");
    debug_print_cell(&sheet->cells[0][15]);

    // Test MAX function
    printf("\nTesting MAX function...\n");
    char max_cmd[] = "Q1=MAX(A1:J1)";
    printf("Processing %s...\n", max_cmd);
    process_command(sheet, max_cmd);
    if (sheet->last_status != STATUS_OK) {
        printf("FAIL: MAX function failed with status %d\n", sheet->last_status);
        teardown(sheet);
        return 0;
    }
    printf("MAX result: ");
    debug_print_cell(&sheet->cells[0][16]);

    // Test MIN function
    printf("\nTesting MIN function...\n");
    char min_cmd[] = "R1=MIN(A1:J1)";
    printf("Processing %s...\n", min_cmd);
    process_command(sheet, min_cmd);
    if (sheet->last_status != STATUS_OK) {
        printf("FAIL: MIN function failed with status %d\n", sheet->last_status);
        teardown(sheet);
        return 0;
    }
    printf("MIN result: ");
    debug_print_cell(&sheet->cells[0][17]);

    // Now test complex formula
    printf("\nTesting complex formula...\n");
    char complex_cmd[] = "S1=O1+P1";  // Sum of SUM and AVG results
    printf("Processing %s...\n", complex_cmd);
    process_command(sheet, complex_cmd);
    if (sheet->last_status != STATUS_OK) {
        printf("FAIL: Complex formula failed with status %d\n", sheet->last_status);
        teardown(sheet);
        return 0;
    }
    printf("Complex formula result: ");
    debug_print_cell(&sheet->cells[0][18]);

    printf("Test completed successfully\n");
    teardown(sheet);
    return 1;
}


//STDEV Checking
int test_statistical_functions() {
    printf("Starting statistical functions test...\n");
    
    struct TestCase {
        const char* setup[5];
        const char* command;
        int expected_value;
        const char* description;
    } tests[] = {
        // STDEV test cases
        {{"A1=10", "A2=20", "A3=30"}, "B1=STDEV(A1:A3)", 8, "Basic STDEV"},
        {{"A1=1", "A2=1", "A3=1"}, "B1=STDEV(A1:A3)", 0, "STDEV of same numbers"},
        {{"A1=0", "A2=100"}, "B1=STDEV(A1:A2)", 50, "STDEV of extremes"},
        {{"A1=2", "A2=4", "A3=4", "A4=4", "A5=6"}, "B1=STDEV(A1:A5)", 1, "STDEV with repeated values"},
        {{"A1=-10", "A2=0", "A3=10"}, "B1=STDEV(A1:A3)", 8, "STDEV with negative numbers"},
        {{"A1=1000", "A2=2000", "A3=3000"}, "B1=STDEV(A1:A3)", 816, "STDEV with large numbers"}
    };

    Spreadsheet* sheet = setup();
    if (!sheet) return 0;

    const size_t num_tests = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        printf("\nTesting %s...\n", tests[i].description);
        
        // Setup cells
        for (size_t j = 0; j < 5 && tests[i].setup[j] != NULL; j++) {
            char cmd[256];
            strncpy(cmd, tests[i].setup[j], sizeof(cmd) - 1);
            cmd[sizeof(cmd) - 1] = '\0';
            process_command(sheet, cmd);
            if (sheet->last_status != STATUS_OK) {
                printf("FAIL: Setup failed for %s\n", tests[i].description);
                teardown(sheet);
                return 0;
            }
        }

        // Execute test command
        char cmd[256];
        strncpy(cmd, tests[i].command, sizeof(cmd) - 1);
        cmd[sizeof(cmd) - 1] = '\0';
        process_command(sheet, cmd);

        // Verify result
        if (sheet->last_status != STATUS_OK) {
            printf("FAIL: Command failed for %s\n", tests[i].description);
            teardown(sheet);
            return 0;
        }

        if (sheet->cells[0][1].value != tests[i].expected_value) {  // B1 is at [0][1]
            printf("FAIL: %s\n", tests[i].description);
            printf("Expected: %d, Got: %d\n",
                   tests[i].expected_value,
                   sheet->cells[0][1].value);
            teardown(sheet);
            return 0;
        }

        printf("PASS: %s\n", tests[i].description);

        // Reset sheet for next test
        teardown(sheet);
        if (i < num_tests - 1) {  // Don't setup after last test
            sheet = setup();
            if (!sheet) return 0;
        }
    }

    printf("All statistical function tests passed\n");
    return 1;
}


//Sleep test
int test_sleep_functionality() {
    printf("Starting sleep functionality test...\n");
    
    struct TestCase {
        const char* setup;
        const char* sleep_cmd;
        const char* description;
        bool expect_sleep;
        int expected_value;
    } tests[] = {
        {"A1=2", "B1=SLEEP(A1)", "Basic sleep", true, 2},
        {"A1=SUM(B1:B2)", "B1=SLEEP(2)", "Sleep with function", true, 2},
        {"A1=0", "B1=SLEEP(A1)", "Sleep with zero", true, 0},
        {"A1=SLEEP(2)", "B1=SUM(A1:A2)", "Function using sleep cell", false, 2}
    };

    Spreadsheet* sheet = setup();
    if (!sheet) return 0;

    const size_t num_tests = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        printf("\nTesting %s...\n", tests[i].description);
        
        // Setup
        if (tests[i].setup) {
            char cmd[256];
            strncpy(cmd, tests[i].setup, sizeof(cmd) - 1);
            cmd[sizeof(cmd) - 1] = '\0';
            process_command(sheet, cmd);
            if (sheet->last_status != STATUS_OK) {
                printf("FAIL: Setup failed for %s\n", tests[i].description);
                teardown(sheet);
                return 0;
            }
        }

        // Execute sleep command
        char cmd[256];
        strncpy(cmd, tests[i].sleep_cmd, sizeof(cmd) - 1);
        cmd[sizeof(cmd) - 1] = '\0';
        
        // Record start time
        struct timeval start_time, end_time;
        gettimeofday(&start_time, NULL);
        
        process_command(sheet, cmd);
        
        gettimeofday(&end_time, NULL);
        double elapsed = (end_time.tv_sec - start_time.tv_sec) +
                        (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

        // Verify command executed successfully
        if (sheet->last_status != STATUS_OK) {
            printf("FAIL: Sleep command failed for %s\n", tests[i].description);
            teardown(sheet);
            return 0;
        }

        // Verify sleep flag based on expectation
        if (tests[i].expect_sleep && !sheet->cells[0][1].is_sleep) {
            printf("FAIL: Sleep flag not set for %s\n", tests[i].description);
            teardown(sheet);
            return 0;
        } else if (!tests[i].expect_sleep && sheet->cells[0][1].is_sleep) {
            printf("FAIL: Sleep flag incorrectly set for %s\n", tests[i].description);
            teardown(sheet);
            return 0;
        }
        
        // Verify cell value
        if (sheet->cells[0][1].value != tests[i].expected_value) {
            printf("FAIL: Incorrect value for %s\n", tests[i].description);
            printf("Expected: %d, Got: %d\n", tests[i].expected_value, sheet->cells[0][1].value);
            teardown(sheet);
            return 0;
        }

        printf("PASS: %s (Elapsed time: %.2f seconds)\n",
               tests[i].description, elapsed);

        // Reset sheet for next test
        teardown(sheet);
        if (i < num_tests - 1) {
            sheet = setup();
            if (!sheet) return 0;
        }
    }

    printf("All sleep functionality tests passed\n");
    return 1;
}


// Range Operations
int test_range_operations() {
    struct TestCase {
        const char* setup[5];
        const char* command;
        const char* description;
    } tests[] = {
        // Valid ranges
        {{"A1=1", "B1=2"}, "C1=SUM(A1:B1)", "Adjacent cells"},
        {{"A1=1", "Z1=26"}, "AA1=SUM(A1:Z1)", "Wide range"},
        {{"A1=1", "A100=100"}, "B1=AVG(A1:A100)", "Tall range"},
        
        // Edge cases
        {{"A1=1"}, "B1=SUM(A1:A1)", "Single cell range"},
        {{"A1=1"}, "B1=SUM(ZZZ1:ZZZ2)", "Range beyond limits"},
        {{"A1=1"}, "B1=SUM(A1:A0)", "Invalid range order"}
    };

    Spreadsheet* sheet = setup_with_size(999, 1000);  // Large sheet for range tests
    if (!sheet) return 0;

    const size_t num_tests = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        printf("\nTesting %s...\n", tests[i].description);
        
        // Setup cells
        for (size_t j = 0; tests[i].setup[j] != NULL && j < 5; j++) {
            char cmd[256];
            strncpy(cmd, tests[i].setup[j], sizeof(cmd) - 1);
            cmd[sizeof(cmd) - 1] = '\0';
            process_command(sheet, cmd);
            if (sheet->last_status != STATUS_OK) {
                printf("FAIL: Setup failed for %s\n", tests[i].description);
                teardown(sheet);
                return 0;
            }
        }

        // Execute test command
        char cmd[256];
        strncpy(cmd, tests[i].command, sizeof(cmd) - 1);
        cmd[sizeof(cmd) - 1] = '\0';
        process_command(sheet, cmd);

        // Verify result based on test case
        switch(i) {
            case 0: // Adjacent cells
                ASSERT(sheet->cells[0][2].value == 3, "Adjacent cells sum failed");
                break;
            case 1: // Wide range
                ASSERT(sheet->cells[0][26].value == 27, "Wide range sum failed");
                break;
            case 2: // Tall range
                ASSERT(sheet->cells[0][1].value == 1, "Tall range average failed");
                break;
            case 3: // Single cell
                ASSERT(sheet->cells[0][1].value == 1, "Single cell sum failed");
                break;
            case 4: // Beyond limits
                ASSERT(sheet->last_status != STATUS_OK, "Range limit check failed");
                break;
            case 5: // Invalid order
                ASSERT(sheet->last_status == ERR_SYNTAX, "Range order check failed");
                break;
        }

        printf("PASS: %s\n", tests[i].description);

        // Reset sheet for next test
        teardown(sheet);
        if (i < num_tests - 1) {
            sheet = setup_with_size(999, 1000);
            if (!sheet) return 0;
        }
    }

    return 1;
}


//Combined test operations
int test_combined_operations() {
    struct TestCase {
        const char* setup[6];
        const char* command;
        const char* description;
    } tests[] = {
        // Sleep with functions
        {{"A1=1", "C1=5"}, "B1=SLEEP(6)", "Complex sleep interaction"},
        
         // Circular with functions
         {{"A1=SLEEP(B1)"}, "B1=A1+1", "Circular sleep dependency"}
        };

    Spreadsheet* sheet = setup();
    if (!sheet) return 0;

    const size_t num_tests = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        printf("\nTesting %s...\n", tests[i].description);
        
        // Setup cells
        for (size_t j = 0; tests[i].setup[j] != NULL && j < 5; j++) {
            char cmd[256];
            strncpy(cmd, tests[i].setup[j], sizeof(cmd) - 1);
            cmd[sizeof(cmd) - 1] = '\0';
            process_command(sheet, cmd);
        }

        // Execute test command
        char cmd[256];
        strncpy(cmd, tests[i].command, sizeof(cmd) - 1);
        cmd[sizeof(cmd) - 1] = '\0';
        process_command(sheet, cmd);

        // Verify result based on test case
        switch(i) {
            case 0: // Sleep with functions
                ASSERT(sheet->cells[0][1].is_sleep, "Sleep property not set");
                break;
        
            case 1: // Circular with functions
                ASSERT(sheet->last_status == ERR_CIRCULAR_REFERENCE, "Circular sleep dependency not detected");
                break;
        }

        printf("PASS: %s\n", tests[i].description);

        // Reset sheet for next test
        teardown(sheet);
        if (i < num_tests - 1) {
            sheet = setup();
            if (!sheet) return 0;
        }
    }

    return 1;
}


//Edge cases
int test_edge_cases() {
    struct TestCase {
        const char* command;
        CalcStatus expected_status;
        const char* description;
    } tests[] = {
        {"A1=SUM()", ERR_SYNTAX, "Empty function"},
        {"A1=SUM(A1:ZZZ99999)", ERR_SYNTAX, "Range too large"},
        {"A1=SLEEP(-1)", STATUS_OK, "Negative sleep handles gracefully"},        
        {"A1=STDEV(B1)", ERR_SYNTAX, "STDEV single cell"},
        {"A1=SUM(AAA111:AAA222)", ERR_SYNTAX, "Invalid cell reference"},
        {"A1=1/0", STATUS_OK, "Direct division by zero"},
    };

    Spreadsheet* sheet = setup();
    if (!sheet) return 0;

    const size_t num_tests = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        printf("\nTesting %s...\n", tests[i].description);
        
        // Execute test command
        char cmd[256];
        strncpy(cmd, tests[i].command, sizeof(cmd) - 1);
        cmd[sizeof(cmd) - 1] = '\0';
        process_command(sheet, cmd);

        // Verify error status
        if (sheet->last_status != tests[i].expected_status) {
            printf("FAIL: %s\n", tests[i].description);
            printf("Expected status: %d, Got: %d\n",
                   tests[i].expected_status, sheet->last_status);
            teardown(sheet);
            return 0;
        }

        // Verify error flag is set only for runtime errors, not syntax errors
        if (!sheet->cells[0][0].has_error &&
            tests[i].expected_status != STATUS_OK && 
            tests[i].expected_status != ERR_SYNTAX) {
            printf("FAIL: Error flag not set for %s\n", tests[i].description);
            teardown(sheet);
            return 0;
        }

        printf("PASS: %s\n", tests[i].description);

        // Reset sheet for next test
        teardown(sheet);
        if (i < num_tests - 1) {
            sheet = setup();
            if (!sheet) return 0;
        }
    }

    return 1;
}



int main() {
    printf("Starting tests...\n\n");
    
    struct TestCategory {
        const char* name;
        int (*test_func)();
    } categories[] = {
        {"Basic Cell Operations", test_basic_cell_operations},
        {"Arithmetic Operations", test_arithmetic_operations},
        {"Spreadsheet Functions", test_spreadsheet_functions},
        {"Division by Zero", test_division_by_zero},
        {"Circular Dependencies", test_circular_dependencies},
        {"Formula Evaluation", test_formula_evaluation},
        {"Scroll Commands", test_scroll_commands},
        {"Advanced Operations", test_advanced_operations},
        {"Statistical Functions", test_statistical_functions},
        {"Sleep Functionality", test_sleep_functionality},
        {"Range Operations", test_range_operations},
        {"Combined Operations", test_combined_operations},
        {"Edge Cases", test_edge_cases},





    };

    const size_t total_categories = sizeof(categories)/sizeof(categories[0]);
    size_t passed = 0;

    for (size_t i = 0; i < total_categories; i++) {
        printf("\nRunning %s tests...\n", categories[i].name);
        
        // Add error handling for each test
        int result = 0;
        
        // Run test with error handling
        __attribute__((unused)) int unused = fflush(stdout);
        result = categories[i].test_func();
        
        if (result) {
            passed++;
            printf("%s: PASSED\n", categories[i].name);
        } else {
            printf("%s: FAILED\n", categories[i].name);
        }
    }

    printf("\nTest Results: %zu/%zu categories passed\n", passed, total_categories);
    return (passed == total_categories) ? 0 : 1;
}