#pragma once

class ElectionResultTemplate {
public:
	virtual ElectionResultTemplate& operator+=(const ElectionResultTemplate*) { return *this; };
	virtual ElectionResultTemplate& operator*=(size_t N) { return *this; };
	virtual void post_process() {};
};

template<class Agent>
class ElectionTemplate {
public:
	virtual ElectionResultTemplate* get_neutral_election_result() const { return NULL; };
	virtual ElectionResultTemplate* operator()(const Agent&) const { return NULL; };
};

template<class Agent>
class ElectionRetroinfluenceTemplate {
public:
	virtual void operator()(Agent&, const ElectionResultTemplate*) const {};
};