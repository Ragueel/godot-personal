#include "sentry_error.h"


String SentryError::get_error_message() {
	Array values{};
	values.append(file);
	values.append(line);
	values.append(function);

	return String("%s::%s::%s").sprintf(values, false);
}

String SentryError::get_simplified_error_message() {
	Array values{};
	values.append(function);
	values.append(rationale);
	values.append(code);

	return String("%s::%s %s").sprintf(values, false);
}
