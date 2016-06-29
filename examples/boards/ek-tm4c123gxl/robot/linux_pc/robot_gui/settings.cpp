
#include "settings.h"


Settings::Pid::Pid()
{
    pidSettings[Pid::proportional] = PidEntry();
    pidSettings[Pid::integral] = PidEntry();
    pidSettings[Pid::derivative] = PidEntry();
}
