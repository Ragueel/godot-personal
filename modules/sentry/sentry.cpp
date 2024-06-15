#include "sentry.h"

#include "scene/main/node.h"


ErrPayload::ErrPayload() {
}

SentryLogger::SentryLogger() = default;

void SentryLogger::logv(const char *p_format, va_list p_list, bool p_err) {
}

void SentryLogger::log_error(const char *p_function, const char *p_file, int p_line, const char *p_code, const char *p_rationale, bool p_editor_notify, ErrorType p_type) {
	Logger::log_error(p_function, p_file, p_line, p_code, p_rationale, p_editor_notify, p_type);

	print_line("Got message");
	if (not_processed_error_payloads.size() > 100) {
		print_line("skipping error");
		return;
	}
	mutex.lock();
	// possible race conditions maybe should use new thread or some sort of mutex
	not_processed_error_payloads.append(memnew(ErrPayload(p_function, p_file, p_line, p_code, p_rationale, p_editor_notify, p_type)));
	mutex.unlock();
}

TypedArray<ErrPayload> SentryLogger::get_not_processed_error_payloads() {
	return not_processed_error_payloads;
}

void SentryLogger::clear_not_processed() {
	mutex.lock();
	not_processed_error_payloads.clear();
	mutex.unlock();
}

Sentry *Sentry::singleton = nullptr;

Sentry::Sentry() {
	if (singleton != nullptr) {
		return;
	}
	singleton = this;
	logger = memnew(SentryLogger);
	sentry_client = memnew(SentryClient);
}

Sentry::~Sentry() {
	if (worker_thread->is_started()) {
		mutex.lock();
		is_running = false;
		mutex.unlock();
		worker_thread->wait_to_finish();
	}
}

void Sentry::set_dsn(String dsn) {
	sentry_dsn = dsn;
	sentry_client->initiailize(dsn);

	mutex.lock();
	is_running = true;
	mutex.unlock();

	worker_thread.instantiate();
	worker_thread->start(callable_mp(this, &Sentry::process_pending_messages), core_bind::Thread::PRIORITY_LOW);
}

void Sentry::execute_in_transaction(StringName transaction_name) {
}

void Sentry::execute_in_span(StringName span_name) {
}

Sentry *Sentry::get_singleton() {
	return Sentry::singleton;
}

void Sentry::process_pending_messages() {
	print_line("consuming messages");
	while (is_running) {
		OS::get_singleton()->delay_usec(1000000);
		TypedArray<ErrPayload> not_processed_payloads = logger->get_not_processed_error_payloads();
		TypedArray<SentryError> sentry_errors{};

		for (int i = 0; i < not_processed_payloads.size(); i++) {
			ErrPayload *current_payload = ErrPayload::cast_to<ErrPayload>(not_processed_payloads.get(i));
			SentryError *err = memnew(SentryError(
				current_payload->p_function,
				current_payload->p_file,
				current_payload->p_line,
				current_payload->p_code,
				current_payload->p_rationale,
				logger_error_type_string(current_payload->p_type)
			));

			sentry_errors.push_back(err);
		}
		logger->clear_not_processed();

		for (int i = 0; i < sentry_errors.size(); i++) {
			sentry_client->send_error(cast_to<SentryError>(sentry_errors.get(i)));
		}

		print_line("Converted to sentry errors now sending it to sentry");
	}
}

String Sentry::logger_error_type_string(Logger::ErrorType error) {
	switch (error) {
		case Logger::ERR_ERROR:
			return "error";
		case Logger::ERR_WARNING:
			return "warning";
		case Logger::ERR_SCRIPT:
			return "script";
		case Logger::ERR_SHADER:
			return "shader";
		default:
			return "unknown";
	}
}

void Sentry::_notification(int p_what) {
	print_line("Something got here");
}

String Sentry::get_dsn() {
	return sentry_dsn;
}

void Sentry::_bind_methods() {
	ClassDB::bind_static_method("Sentry", D_METHOD("get_singleton"), &Sentry::get_singleton);
	ClassDB::bind_method(D_METHOD("set_dsn", "dsn"), &Sentry::set_dsn);
	ClassDB::bind_method(D_METHOD("get_dsn"), &Sentry::get_dsn);
	// ClassDB::bind_method(D_METHOD("start_transaction"), &Sentry::start_transaction);
	// ClassDB::bind_method(D_METHOD("start_span"), &Sentry::start_span);
}
