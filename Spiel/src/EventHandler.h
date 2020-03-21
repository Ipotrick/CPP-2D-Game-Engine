#include <string>
#include <functional>

using eventTriggerFuncPtrType = void (*)(std::string_view, uint32_t);

struct EventSubscribtion {
	EventSubscribtion(std::string eventName_, eventTriggerFuncPtrType onEventTrigger_, uint32_t id_) :
		eventName{ eventName_ },
		onEventTrigger{ onEventTrigger_},
		id{ id_ }
	{}

	std::string eventName;
	eventTriggerFuncPtrType onEventTrigger;
	uint32_t id;
};

class EventHandler {
public:
	void triggerEvent(std::string eventName);
	void subscribeToEvent(std::string eventName, eventTriggerFuncPtrType onEventTrigger, uint32_t id);
	void unSubscribeEvent(std::string eventName, eventTriggerFuncPtrType onEventTrigger, uint32_t id);
private:
	std::unordered_map<std::string, std::vector<EventSubscribtion>> eventMap;
};