#include "EventHandler.h"

bool operator==(EventSubscribtion const& a, EventSubscribtion const& b) {
	return a.id == b.id && a.onEventTrigger == b.onEventTrigger;
}

void EventHandler::triggerEvent(std::string_view eventName) {
	auto event = eventMap.find(eventName);

	if (event != eventMap.end()) {
		for (auto iter = event->second.begin(); iter != event->second.end();) {
			bool unsubscribe = iter->onEventTrigger(eventName, iter->id);
			if (unsubscribe) {
				iter = event->second.erase(iter);
			}
			else {
				++iter;
			}
		}
		if (event->second.size() == 0) {
			eventMap.erase(event);
		}
	}
}

void EventHandler::subscribeToEvent(std::string_view eventName, eventTriggerFuncPtrType onEventTrigger, uint32_t id) {
	//look if the event allready has a subscribtionList:
	auto event = eventMap.find(eventName);
	if (event != eventMap.end()) {
		event->second.push_back(EventSubscribtion(onEventTrigger, id));
	}
	else {	//there is no event sublist yet
		eventMap.insert({ eventName , std::vector<EventSubscribtion>() });
		auto event = eventMap.find(eventName);
		event->second.push_back(EventSubscribtion(onEventTrigger, id));
	}
}

void EventHandler::unSubscribeFromEvent(std::string_view eventName, eventTriggerFuncPtrType onEventTrigger, uint32_t id) {
	auto event = eventMap.find(eventName);
	if (event != eventMap.end()) {
		EventSubscribtion subToDelete = EventSubscribtion(onEventTrigger, id);
		for (auto iter = event->second.begin(); iter != event->second.end(); ++iter) {
			if (*iter == subToDelete) {
				event->second.erase(iter);
				break;
			}
		}
	}
	if (event->second.size() == 0) {
		eventMap.erase(event);
	}
}