#include <chrono>

#include "ecole/reward/solvingtime.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::reward {

const int MUS_IN_SECONDS = 1000000;

static auto process_solving_time(scip::Model const& model) {
	switch (model.get_stage()) {
	// Only stages when the following call is authorized
	case SCIP_STAGE_PROBLEM:
	case SCIP_STAGE_TRANSFORMING:
	case SCIP_STAGE_TRANSFORMED:
	case SCIP_STAGE_INITPRESOLVE:
	case SCIP_STAGE_PRESOLVING:
	case SCIP_STAGE_EXITPRESOLVE:
	case SCIP_STAGE_PRESOLVED:
	case SCIP_STAGE_INITSOLVE:
	case SCIP_STAGE_SOLVING:
	case SCIP_STAGE_SOLVED:
		return SCIPgetSolvingTime(model.get_scip_ptr());
	default:
		return decltype(SCIPgetSolvingTime(nullptr)){0};
	}
}

void SolvingTime::reset(scip::Model& model) {
	model.set_params({{"timing/clocktype", 1}});

	if (wall) {
		auto now_in_mus = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch());
		solving_time_offset = static_cast<double>(now_in_mus.count()) / MUS_IN_SECONDS;
	} else {
		solving_time_offset = 0.;
	}
}

Reward SolvingTime::obtain_reward(scip::Model& model, bool /* done */) {
	double solving_time_diff;

	if (wall) {
		auto now_in_mus = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch());
		solving_time_diff = static_cast<double>(now_in_mus.count()) / MUS_IN_SECONDS - solving_time_offset;
	} else {
		solving_time_diff = static_cast<double>(process_solving_time(model)) - solving_time_offset;
	}
	solving_time_offset += solving_time_diff;
	return solving_time_diff;
}

}  // namespace ecole::reward
