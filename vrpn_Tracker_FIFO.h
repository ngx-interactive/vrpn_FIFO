//#include "vrpn_Configure.h"

//#if defined(VRPN_USE_FIFO)

//#include "vrpn_Analog.h"
//#include "vrpn_Button.h"
#include "vrpn_Tracker.h"
//#include "vrpn_Text.h"

/**
 * This Tracker accepts input from STDIN
 * @Author Philippe Crassous / ENSAM ParisTech-Institut Image
 */
class vrpn_Tracker_FIFO :
  public vrpn_Tracker
{
public:
	vrpn_Tracker_FIFO(
		const char* name,
		vrpn_Connection* c,
		int id
		);

	void mainloop();

private:
	bool _parse_tracker_data();
  void report_changes(float pos[], int id);

};

//#endif // ifdef STDIN