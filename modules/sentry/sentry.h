/* Sentry */

#ifndef SENTRY_H
#define SENTRY_H

#include "client/sentry_client.h"
#include "core/core_bind.h"
#include "core/io/logger.h"

class ErrPayload : public RefCounted {
	GDCLASS(ErrPayload, RefCounted)
public:
	ErrPayload();

	ErrPayload(const char *p_function, const char *p_file, int p_line, const char *p_code, const char *p_rationale, bool p_editor_notify, Logger::ErrorType p_type) :
		p_function(p_function),
		p_file(p_file),
		p_line(p_line),
		p_code(p_code),
		p_rationale(p_rationale),
		p_editor_notify(p_editor_notify),
		p_type(p_type) {
	}

	const char *p_function;
	const char *p_file;
	int p_line;
	const char *p_code;
	const char *p_rationale;
	bool p_editor_notify;
	Logger::ErrorType p_type;
};

class SentryLogger : public Logger {
public:
	SentryLogger();
	void logv(const char *p_format, va_list p_list, bool p_err) override;
	void log_error(const char *p_function, const char *p_file, int p_line, const char *p_code, const char *p_rationale, bool p_editor_notify, ErrorType p_type) override;

	TypedArray<ErrPayload> get_not_processed_error_payloads();
	void clear_not_processed();

private:
	Mutex mutex;
	TypedArray<ErrPayload> not_processed_error_payloads;
};

class Sentry : public Object {
	GDCLASS(Sentry, Object)

public:
	Sentry();
	~Sentry();

	static Sentry *singleton;

	void set_dsn(String dsn);
	String get_dsn();

	void execute_in_transaction(StringName transaction_name);
	void execute_in_span(StringName span_name);

	static Sentry *get_singleton();
	SentryLogger *logger;

private:
	SentryClient *sentry_client;
	Ref<core_bind::Thread> worker_thread;
	Mutex mutex;
	String sentry_dsn;
	bool is_running;

	void process_pending_messages();
	String logger_error_type_string(Logger::ErrorType error);

protected:
	void _notification(int p_notification);
	static void _bind_methods();
};
#endif
