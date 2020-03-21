#include "EventHandler.h"
#include <iostream>

bool operator==(EventSubscribtion const& a, EventSubscribtion const& b) {
	return a.eventName == b.eventName && a.id == b.id && a.onEventTrigger == b.onEventTrigger;
}

void EventHandler::triggerEvent(std::string eventName) {
	auto event = eventMap.find(eventName);

	if (event != eventMap.end()) {
		for (auto eventSubscribtion : event->second) {
			eventSubscribtion.onEventTrigger(eventSubscribtion.eventName, eventSubscribtion.id);
		}
	}
}

void EventHandler::subscribeToEvent(std::string eventName, eventTriggerFuncPtrType onEventTrigger, uint32_t id) {
	//look if the event allready has a subscribtionList:
	auto event = eventMap.find(eventName);
	if (event != eventMap.end()) {
		event->second.push_back(EventSubscribtion(eventName, onEventTrigger, id));
	}
	else {	//there is no event sublist yet
		eventMap.insert({ eventName , std::vector<EventSubscribtion>() });
		auto event = eventMap.find(eventName);
		event->second.push_back(EventSubscribtion(eventName, onEventTrigger, id));
	}
}

void EventHandler::unSubscribeEvent(std::string eventName, eventTriggerFuncPtrType onEventTrigger, uint32_t id) {
	auto event = eventMap.find(eventName);
	if (event != eventMap.end()) {
		EventSubscribtion subToDelete = EventSubscribtion(eventName, onEventTrigger, id);
		for (auto eventSubscribtion : event->second) {
			if (eventSubscribtion == subToDelete) {

			}
		}
	}
}