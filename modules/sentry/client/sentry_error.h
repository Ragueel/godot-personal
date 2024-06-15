#ifndef SENTRY_ERROR_H
#define SENTRY_ERROR_H

#include "core/object/ref_counted.h"

class SentryError : public RefCounted {
public:
	SentryError(String function, String file, int line, String code, String rationale, String error_type) :
		function(function),
		file(file),
		line(line),
		code(code),
		rationale(rationale),
		error_type(error_type) {
	}

	String function;
	String file;
	int line;
	String code;
	String rationale;
	String error_type;
};
#endif
