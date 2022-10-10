/*
 * Event.h
 *
 *  Created on: May 6, 2021
 *      Author: pascal
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <vector>
#include <list>

using namespace std;

static int     seq_nr;

enum Event_type { INVALID_EVENT, SLACK, JOBFINISH, FAULT, BATTERY};
enum faultSubCategory {NO_SUB, CRASH, BABBLING_IDIOT, OMISSION};

namespace Fractal {

class Event {
public:
	int ID;
	int target;
	int eventTime;
	int value;
	Event_type eventType;
	faultSubCategory subCategory;

	//Comparison operator to compare events
    bool operator == (const Event& s) const { return ID == s.ID && target == s.target && eventTime == s.eventTime && value == s.value && eventType == s.eventType && subCategory == s.subCategory; }
    //bool operator != (const Event& s) const { return !operator==(s); }

	Event(int id) : ID(seq_nr++), target(id), value(0), eventTime(0), subCategory(NO_SUB), eventType(INVALID_EVENT){};
	static void free_events();

	void setID(int ID);
	int getID();

	void setEventTime(int time);
	int getEventTime();
	void setEventType(Event_type evenType);
	Event_type getEventType();
};

} /* namespace Fractal */

#endif /* EVENT_H_ */
