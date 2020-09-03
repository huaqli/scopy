#ifndef MIXED_SIGNAL_SINK_H
#define MIXED_SIGNAL_SINK_H

#include <gnuradio/sync_block.h>

#include "logicanalyzer/logic_analyzer.h"
#include "TimeDomainDisplayPlot.h"

class mixed_signal_sink : virtual public gr::sync_block
{
public:
	typedef boost::shared_ptr<mixed_signal_sink> sptr;

	static sptr make(adiscope::logic::LogicAnalyzer *logicAnalyzer,
	                 adiscope::TimeDomainDisplayPlot *oscPlot,
	                 int bufferSize);

	virtual void clean_buffers() = 0;
	virtual void set_nsamps(int newsize) = 0;
};

#endif // MIXED_SIGNAL_SINK_H
