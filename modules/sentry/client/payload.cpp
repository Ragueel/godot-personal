#include "payload.h"

#include "core/io/json.h"

Dictionary SentrySdkInfo::to_sentry_payload() {
	Dictionary payload{};
	payload["version"] = version;
	payload["name"] = name;

	return payload;
}

Dictionary EnvelopeHeaders::to_sentry_payload() {
	Dictionary payload{};
	payload["sdk"] = sdk->to_sentry_payload();
	payload["request_id"] = request_id;
	payload["dsn"] = dsn;
	payload["sent_at"] = sent_at;

	return payload;
}

Dictionary EnvelopeErrorMessage::to_sentry_payload() {
	Dictionary payload{};
	payload["type"] = type;
	payload["path"] = path;
	payload["details"] = details;

	return payload;
}

String EnvelopeMessage::get_level_string() {
	switch (level) {
		case FATAL:
			return "fatal";
		case ERROR:
			return "error";
		case WARNING:
			return "warning";
		case INFO:
			return "info";
		case DEBUG:
			return "debug";
		default:
			return "error";
	}
}

Dictionary EnvelopeMessage::to_sentry_payload() {
	TypedArray<Dictionary> errors{};

	for (int i = 0; i < message_errors.size(); i++) {
		EnvelopeErrorMessage *error_message = cast_to<EnvelopeErrorMessage>(message_errors[i]);
		errors.append(error_message->to_sentry_payload());
	}

	Dictionary payload{};
	payload["message"] = message;
	payload["logger"] = logger;
	payload["extra"] = extra;
	payload["level"] = get_level_string();
	payload["errors"] = errors;
	payload["platform"] = "native";
	payload["timestamp"] = Time::get_singleton()->get_datetime_string_from_system(true, false) + "Z";
	payload["fingerprint"] = fingerprint;

	Dictionary tags{};
	tags["os"] = OS::get_singleton()->get_name();
	tags["model_name"] = OS::get_singleton()->get_model_name();
	payload["tags"] = tags;

	Dictionary contexts{};
	PackedStringArray gpu_info = OS::get_singleton()->get_video_adapter_driver_info();
	// Only works on windows/linux
	// I think it is possible to get this info from somewhere else
	if (gpu_info.size() > 1) {
		Dictionary gpu_context{};
		gpu_context["name"] = gpu_info[0];
		gpu_context["version"] = gpu_context[1];
		contexts["gpu"] = gpu_context;
	}
	payload["contexts"] = contexts;

	Dictionary log_entry{};
	log_entry["formatted"] = log_message;
	payload["logentry"] = log_entry;

	return payload;
}

String EnvelopeItem::to_sentry_payload() {
	Dictionary item{};
	item["content_type"] = this->content_type;
	item["type"] = get_type_as_string();
	item["filename"] = filename;

	String result = JSON::stringify(item);

	for (int i = 0; i < messages.size(); i++) {
		result += "\n";
		EnvelopeMessage *message = cast_to<EnvelopeMessage>(messages[i]);
		result += JSON::stringify(message->to_sentry_payload());
	}

	return result;
}

String EnvelopeItem::get_type_as_string() {
	switch (type) {
		case EVENT:
			return "event";
		default:
			return "event";
	}
}

String Envelope::to_sentry_payload() {
	String headers = JSON::stringify(env_headers->to_sentry_payload());
	for (int i = 0; i < items.size(); i++) {
		EnvelopeItem *item = cast_to<EnvelopeItem>(items.get(i));
		headers += "\n";
		headers += item->to_sentry_payload();
	}
	return headers;
}
