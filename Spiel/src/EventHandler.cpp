#include "EventHandler.h"

bool operator==(EventSubscribtion const& a, EventSubscribtion const& b) {
	return a.onEventTrigger == b.onEventTrigger && a.data == b.data;
}

void EventHandler::triggerEvent(std::string_view eventName) {
	auto event = eventMap.find(eventName);

	if (event != eventMap.end()) {
		for (auto iter = event->second.begin(); iter != event->second.end();) {
			bool unsubscribe = iter->onEventTrigger(eventName, iter->data); 
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

void EventHandler::subscribeToEvent(std::string_view eventName, eventTriggerFuncPtrType onEventTrigger, void* data) {
	//look if the event allready has a subscribtionList:
	auto event = eventMap.find(eventName);
	if (event != eventMap.end()) {
		event->second.push_back(EventSubscribtion(onEventTrigger, data));
	}
	else {	//there is no event sublist yet
		eventMap.insert({ eventName , std::vector<EventSubscribtion>() });
		auto event = eventMap.find(eventName);
		event->second.push_back(EventSubscribtion(onEventTrigger, data));
	}
}

void EventHandler::unSubscribeFromEvent(std::string_view eventName, eventTriggerFuncPtrType onEventTrigger, void* data) {
	auto event = eventMap.find(eventName);
	if (event != eventMap.end()) {
		EventSubscribtion subToDelete = EventSubscribtion(onEventTrigger, data);
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