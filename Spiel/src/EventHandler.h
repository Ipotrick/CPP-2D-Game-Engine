#include <string>
#include <string_view>
#include <functional>

/*  an event function MUST be static or free.
	the input types for an event function are (event name, id(variable use)) and the return type is a bool.
	the return bool is used to specify if the function wants to be unsubscribed right after beeing called in a trigger.
	return true : unsubscribe after call, return false : keep me subscribed*/
using eventTriggerFuncPtrType = bool (*)(std::string_view, void*);

struct EventSubscribtion {
	EventSubscribtion(eventTriggerFuncPtrType onEventTrigger_, void* data_) :
		onEventTrigger{ onEventTrigger_},
		data{ data_ }
	{}

	eventTriggerFuncPtrType onEventTrigger;
	void* data;
};

class EventHandler {
public:
	/* trigger an event. this will call all subscribed functions under the eventname */
	void triggerEvent(std::string_view eventName);
	/* subscribe a function to react to an event name */
	void subscribeToEvent(std::string_view eventName, eventTriggerFuncPtrType onEventTrigger, void*);
	/* unsubscribe a function to an event name with an id */
	void unSubscribeFromEvent(std::string_view eventName, eventTriggerFuncPtrType onEventTrigger, void*);
private:
	std::unordered_map<std::string_view, std::vector<EventSubscribtion>> eventMap;
};