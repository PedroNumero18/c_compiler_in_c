/**
 * Error Handling Implementation
 * 
 * Implements functions for reporting and handling errors
 * during compilation.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include "error.h"
 
 // Count of errors encountered
 static int error_count = 0;
 
 // Report an error without location information
 void error_report(const char *filename, const char *message) {
     if (filename) {
         fprintf(stderr, "Error in %s: %s\n", filename, message);
     } else {
         fprintf(stderr, "Error: %s\n", message);
     }
     error_count++;
 }
 
 // Report an error with line and column information
 void error_report_location(const char *filename, int line, int column, const char *message) {
     if (filename) {
         fprintf(stderr, "Error in %s:%d:%d: %s\n", filename, line, column, message);
     } else {
         fprintf(stderr, "Error at line %d, column %d: %s\n", line, column, message);
     }
     error_count++;
 }
 
 // Report an error with token information
 void error_report_with_token(const char *filename, int line, int column, const char *token, const char *message) {
     if (filename) {
         fprintf(stderr, "Error in %s:%d:%d: %s: '%s'\n", filename, line, column, message, token);
     } else {
         fprintf(stderr, "Error at line %d, column %d: %s: '%s'\n", line, column, message, token);
     }
     error_count++;
 }
 
 // Get the total number of errors encountered
 int get_error_count() {
     return error_count;
 }
 
 // Reset the error count
 void reset_error_count() {
     error_count = 0;
 }
 