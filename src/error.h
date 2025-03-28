/**
 * Error Handling Header
 * 
 * Defines functions for reporting and handling errors
 * during compilation.
 */

 #ifndef ERROR_H
 #define ERROR_H
 
 // Function prototypes
 void error_report(const char *filename, const char *message);
 void error_report_location(const char *filename, int line, int column, const char *message);
 void error_report_with_token(const char *filename, int line, int column, const char *token, const char *message);
 
 // Get total error count
 int get_error_count();
 
 // Reset error count
 void reset_error_count();
 
 #endif // ERROR_H
 