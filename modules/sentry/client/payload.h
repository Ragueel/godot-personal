#ifndef SENTRY_PAYLOAD_H
#define SENTRY_PAYLOAD_H

#include "core/object/ref_counted.h"
#include "core/os/time.h"
#include "core/variant/typed_array.h"


class SentrySdkInfo : public RefCounted {
public:
	SentrySdkInfo(const StringName &version, const StringName &name) :
		version(version),
		name(name) {
	}

	StringName version;
	StringName name;

	Dictionary to_sentry_payload();
};

class EnvelopeHeaders : public RefCounted {
public:
	EnvelopeHeaders(const String &request_id, const String &dsn, SentrySdkInfo *sdk, String sent_at) :
		request_id(request_id),
		dsn(dsn),
		sdk(sdk),
		sent_at(sent_at) {
	}

	String request_id;
	String dsn;
	SentrySdkInfo *sdk;
	String sent_at;

	Dictionary to_sentry_payload();
};

class EnvelopeErrorMessage : public RefCounted {
public:
	EnvelopeErrorMessage(const String &type, const String &path, const String &details) :
		type(type),
		path(path),
		details(details) {
	}

	String type;
	String path;
	String details;
	Dictionary to_sentry_payload();
};

class EnvelopeMessage : public RefCounted {
public:
	enum Level {
		FATAL,
		ERROR,
		WARNING,
		INFO,
		DEBUG,
	};

	EnvelopeMessage(::EnvelopeMessage::Level level, const String &message, const String &log_message, const String &logger, const Dictionary &extra, const TypedArray<EnvelopeErrorMessage> &errors, Vector<String> fingerprint) :
		message(message),
		log_message(log_message),
		level(level),
		logger(logger),
		extra(extra),
		message_errors(errors),
		fingerprint(fingerprint) {
	}

	String message;
	String log_message;
	Level level;
	String logger;
	Dictionary extra;
	TypedArray<EnvelopeErrorMessage> message_errors;
	Vector<String> fingerprint;

	String get_level_string();
	Dictionary to_sentry_payload();
};

class EnvelopeItem : public RefCounted {
public:
	enum EnvelopeType {
		EVENT,
		TRANSACTION,
		ATTACHMENT,
		SESSION,
		STATSD,
		METRIC_META,
		USER_REPORT,
	};

	EnvelopeItem(EnvelopeType type, const String &filename, TypedArray<EnvelopeMessage> messages) :
		type(type),
		content_type("application/json"),
		filename(filename),
		messages(messages) {
	}

	EnvelopeType type;
	String content_type;
	String filename;
	TypedArray<EnvelopeMessage> messages;

	String to_sentry_payload();
	String get_type_as_string();
};

class Envelope : public RefCounted {
public:
	Envelope(EnvelopeHeaders *headers, const TypedArray<EnvelopeItem> &items) :
		env_headers(headers),
		items(items) {
	}

	EnvelopeHeaders *env_headers;
	TypedArray<EnvelopeItem> items;

	String to_sentry_payload();
};

#endif
