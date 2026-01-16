// error.h
//
#ifndef _SUS_ERROR_
#define _SUS_ERROR_

#include "core.h"

/*
* 
* SUS_NOTUSE_ERRORMANAGER Do not use the error manager
*/

// -------------------------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)
#pragma pack(push, 1)
// The type of severity of the error
typedef enum sus_error_severity {
	SUS_ERROR_SEVERITY_COMMON,	// A common error, The function returns invalid data
	SUS_ERROR_SEVERITY_WARNING,	// Warning, we continue to work
	SUS_ERROR_SEVERITY_CRITICAL,// Critical error, need to change program execution\terminate program
	SUS_ERROR_SEVERITY_FATAL	// Fatal error, requires program termination
} SUS_ERROR_SEVERITY;
// Type of error
typedef enum sus_error_type {
	SUS_ERROR_TYPE_GENERAL,
	SUS_ERROR_TYPE_SYSTEM,
	SUS_ERROR_TYPE_MEMORY,
	SUS_ERROR_TYPE_IO,
	SUS_ERROR_TYPE_PARSER,
	SUS_ERROR_TYPE_RESOURCE,
	SUS_ERROR_TYPE_USER,
} SUS_ERROR_TYPE;
// Error structure
typedef union sus_error {
	sus_uint32_t value;
	struct {
		sus_uint32_t sev : 8;	// Severity of the error
		sus_uint32_t type : 8;	// Type of error
		sus_uint32_t code : 16;	// Error code
	};
} SUS_ERROR;
#pragma pack(pop)
#pragma warning(pop)

// -------------------------------------------------------------------------------------------------

// Handler for critical and fatal errors
typedef BOOL(SUSAPI* SUS_ERROR_HANDLER)(SUS_ERROR error);
// Error stack size
#define SUS_ERROR_STACK_SIZE 64
// Size of the stack of handler functions
#define SUS_ERROR_HANDLER_STACK_SIZE 16

#ifndef SUS_NOTUSE_ERRORMANAGER

// -------------------------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)
// Error Manager
static struct sus_error_manager {
	struct {
		SUS_ERROR		stack[SUS_ERROR_STACK_SIZE];
		sus_uint32_t	sp;	// Position on the stack
	} error;
	struct {
		SUS_ERROR_HANDLER	stack[SUS_ERROR_HANDLER_STACK_SIZE];
		sus_uint32_t		sp;	// Position on the stack
	} handler;
} SUSErrorManager = { 0 };
#pragma warning(pop)

// -------------------------------------------------------------------------------------------------

// Install the error handler\param handler != 0 -> push\param handler == 0 -> pop
SUS_INLINE VOID SUSAPI susErrorSetHandler(_In_opt_ SUS_ERROR_HANDLER handler) {
	SUS_ASSERT(!handler ? SUSErrorManager.handler.sp : TRUE);
	if (!handler) SUSErrorManager.handler.sp--;
	SUSErrorManager.handler.stack[SUSErrorManager.handler.sp++] = handler;
}
// Set the error code\return TRUE - Continue working\return FALSE - Collapse the stack
SUS_INLINE BOOL SUSAPI susErrorPushEx(_In_ SUS_ERROR error) {
	SUS_ASSERT(SUSErrorManager.error.sp < SUS_ERROR_STACK_SIZE);
	SUSErrorManager.error.stack[SUSErrorManager.error.sp++] = error;
	SUS_PRINTDL("Error code %d has been set", error.value);
	if (SUSErrorManager.handler.sp) return SUSErrorManager.handler.stack[SUSErrorManager.handler.sp](error);
	return TRUE;
}
// Create a simple error
#define susErrorPush(code_, type_) susErrorPushEx((SUS_ERROR) { .code = code_, .sev = SUS_ERROR_SEVERITY_COMMON, .type = type_ });
// Create a critical error
#define susErrorPushCritical(code_, type_) susErrorPushEx((SUS_ERROR) { .code = code_, .sev = SUS_ERROR_SEVERITY_CRITICAL, .type = type_ });
// Create a fatal error
#define susErrorPushFatal(code_, type_) susErrorPushEx((SUS_ERROR) { .code = code_, .sev = SUS_ERROR_SEVERITY_FATAL, .type = type_ });
// Peek at the error
SUS_INLINE SUS_ERROR SUSAPI susErrorPeek() {
	if (!SUSErrorManager.error.sp) return (SUS_ERROR) { 0 };
	return SUSErrorManager.error.stack[SUSErrorManager.error.sp];
}
// Extract the error
SUS_INLINE SUS_ERROR SUSAPI susErrorPop() {
	if (!SUSErrorManager.error.sp) return (SUS_ERROR) { 0 };
	return SUSErrorManager.error.stack[SUSErrorManager.error.sp--];
}

// -------------------------------------------------------------------------------------------------

#else

#define susErrorSetHandler(handler)
#define susErrorPush(error) (TRUE)
#define susErrorPeek() (SUS_ERROR) { 0 }
#define susErrorPop() (SUS_ERROR) { 0 }

#endif /* !SUS_NOTUSE_ERRORMANAGER */

// -------------------------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)
// Kernel Error Codes
enum {
	SUS_ERROR_SUCCESS = 0x0000,		// There are no errors
	SUS_ERROR_INVALID_PARAM,		// Incorrect parameter
	SUS_ERROR_INVALID_STATE,		// Wrong context
	SUS_ERROR_INVALID_OPERATION,	// Invalid operation from the context
	SUS_ERROR_INVALID_INPUT,		// Incorrect data was received
	SUS_ERROR_WRITE_MEMORY,			// Couldn't write data
	SUS_ERROR_OUT_OF_MEMORY,		// Memory overflow
	SUS_ERROR_OUT_OF_RANGE,			// Going beyond the boundaries of access
	SUS_ERROR_STACK_OVERFLOW,		// Static stack overflow
	SUS_ERROR_SYNTAX_ERROR,			// Error during parsing
	SUS_ERROR_SYSTEM_ERROR,			// System error
	SUS_ERROR_API_ERROR,			// System error
	SUS_ERROR_USER1					// User errors
};
#pragma warning(pop)

// -------------------------------------------------------------------------------------------------

#endif // !_SUS_ERROR_
