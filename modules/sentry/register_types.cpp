#include "sentry.h"
#include "core/object/class_db.h"
#include "core/os/os.h"
#include "modules/register_module_types.h"
#include "client/sentry_client.h"

void initialize_sentry_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_CORE) {
		return;
	}
	ClassDB::register_class<Sentry>();
	ClassDB::register_class<ErrPayload>();

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	Sentry *global_instance = memnew(Sentry);
	OS::get_singleton()->add_logger_from_outside(global_instance->logger);
}

void uninitialize_sentry_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_CORE) {
		return;
	}
	// TODO: Do something here
}
