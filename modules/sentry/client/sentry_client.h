#ifndef SENTRY_CLIENT_H
#define SENTRY_CLIENT_H

#include "payload.h"
#include "sentry_error.h"
#include "core/io/http_client.h"
#include "core/object/object.h"
#include "editor/export/export_template_manager.h"

class SentryClient : public Object {
	GDCLASS(SentryClient, Object)
private:
	Ref<Crypto> crypto;

	bool is_ready;
	String dsn;
	String envelope_endpoint;

	Mutex requests_pool_mutex;
	TypedArray<HTTPRequest *> requests_pool;

	HTTPRequest *get_request_from_pool();
	void put_request_into_pool(HTTPRequest *request);

public:
	SentryClient();
	void initiailize(String sentry_dsn);
	void close();
	bool is_initialized();

	void send_error(SentryError *error);
	void send_payload(Vector<String> &headers, Vector<uint8_t> &p_body, String &endpoint);
	void request_completed(int result, int code, Vector<String> headers, PackedByteArray body, const HTTPRequest *request);
	Envelope *envelope_from_error(SentryError *error);

	String random_id();
};
#endif
