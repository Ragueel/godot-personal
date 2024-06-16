#include "sentry_client.h"

#include "core/config/project_settings.h"
#include "core/io/json.h"
#include "scene/main/http_request.h"
#include "scene/main/window.h"


SentryClient::SentryClient() {
	is_ready = false;
}

void SentryClient::initiailize(String sentry_dsn) {
	crypto = Ref<Crypto>(Crypto::create());

	dsn = sentry_dsn;

	// Not so sure about the way api is retrieved
	Vector<String> split_result = dsn.split(":");
	Vector<String> split_by_slash = dsn.split("/");

	String protocol = split_result[0];
	String middle_part = split_by_slash[split_by_slash.size() - 2];

	Vector<String> split_by_dot = middle_part.split(".");
	String domain = "";

	for (int i = 1; i < split_by_dot.size(); i++) {
		if (split_by_dot[i] == "ingest") {
			continue;
		}
		domain += split_by_dot[i];
		if (i != split_by_dot.size() - 1) {
			domain += ".";
		}
	}

	String project_id = split_by_slash[split_by_slash.size() - 1];

	envelope_endpoint = protocol + "://" + domain + "/api/" + project_id + "/envelope/";
}

bool SentryClient::is_initialized() {
	return is_ready;
}

void SentryClient::send_error(SentryError *error) {
	Vector<String> headers{};
	headers.append("Content-Type: multipart/form-data");

	Envelope *envelope = envelope_from_error(error);
	String payload = envelope->to_sentry_payload();
	print_line(payload);

	Vector<uint8_t> p_body = payload.to_utf8_buffer();

	send_payload(headers, p_body, envelope_endpoint);
}

void SentryClient::send_payload(Vector<String> &headers, Vector<uint8_t> &p_body, String &endpoint) {
	SceneTree *tree = SceneTree::get_singleton();
	HTTPRequest *request = memnew(HTTPRequest);

	tree->get_root()->call_deferred("add_child", request);

	request->call_deferred("request_raw", endpoint, headers, HTTPClient::METHOD_POST, p_body);
	request->connect("request_completed", callable_mp(this, &SentryClient::request_completed));
}

void SentryClient::request_completed(int result, int code, Vector<String> headers, PackedByteArray body) {
	String result_json = String::utf8((const char *)&body[0], body.size());
	print_line(result_json);
}

Envelope *SentryClient::envelope_from_error(SentryError *error) {
	SentrySdkInfo *sdk_info = memnew(SentrySdkInfo("0.0.0", "godot"));
	EnvelopeHeaders *headers = memnew(EnvelopeHeaders(random_id(), dsn, sdk_info, Time::get_singleton()->get_datetime_string_from_system(true) +"Z"));
	Dictionary extra{};

	TypedArray<EnvelopeMessage> messages{};

	TypedArray<EnvelopeErrorMessage> errors{};
	errors.append(memnew(EnvelopeErrorMessage(error->error_type, error->get_error_message(), error->code)));
	Vector<String> fingerprint{};
	fingerprint.append(error->function);
	fingerprint.append(error->file);
	fingerprint.append(String::num_int64(error->line));
	fingerprint.append(error->code);

	print_line(error->code);
	EnvelopeMessage *message = memnew(EnvelopeMessage(EnvelopeMessage::Level::ERROR, error->get_simplified_error_message(), error->code, "godot.sentry.logger", extra, errors, fingerprint));
	messages.append(message);

	TypedArray<EnvelopeItem> items{};
	EnvelopeItem *item = memnew(EnvelopeItem(EnvelopeItem::EVENT, "logger.log", messages));
	items.append(item);

	return memnew(Envelope(headers, items));
}

String SentryClient::random_id() {
	PackedByteArray data = crypto->generate_random_bytes(16);

	data.set(6, (data[6] & 0x0f) | 0x40);
	data.set(8, (data[8] & 0x3f) | 0x80);

	return String::hex_encode_buffer(&data[0], 16)
	       .insert(8, "-")
	       .insert(13, "-")
	       .insert(18, "-")
	       .insert(23, "-");
}
