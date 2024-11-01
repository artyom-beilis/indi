#if 0
LX200 Generic
Copyright (C) 2003 - 2017 Jasem Mutlaq (mutlaqja@ikarustech.com)

This library is free software;
you can redistribute it and / or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation;
either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY;
without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library;
if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301  USA

2013 - 10 - 27:
Updated driver to use INDI::Telescope (JM)
2015 - 11 - 25:
    Use variable POLLMS instead of static POLLMS

#endif

#include "lx200generic.h"
#include "lx200_10micron.h"
#include "lx200_16.h"
#include "lx200_OnStep.h"
#include "lx200_OpenAstroTech.h"
#include "lx200ap_v2.h"
#include "lx200classic.h"
#include "lx200fs2.h"
#include "lx200gemini.h"
#include "lx200pulsar2.h"
#include "lx200ss2000pc.h"
#include "lx200zeq25.h"
#include "lx200gotonova.h"
#include "ioptronHC8406.h"
#include "lx200am5.h"
#include "lx200_pegasus_nyx101.h"
#include <cmath>
#include <memory>
#include <cstring>
#include <unistd.h>
#include "eq500x.h"

#if 0
LX200 Generic
Copyright (C) 2003 - 2017 Jasem Mutlaq (mutlaqja@ikarustech.com)

This library is free software;
you can redistribute it and / or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation;
either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY;
without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library;
if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301  USA

2013 - 10 - 27:
Updated driver to use INDI::Telescope (JM)
2015 - 11 - 25:
    Use variable POLLMS instead of static POLLMS

#endif

#include "lx200generic.h"

#include "lx200_10micron.h"
#include "lx200_16.h"
#include "lx200_OnStep.h"
#include "lx200_OpenAstroTech.h"
#include "lx200ap_v2.h"
#include "lx200classic.h"
#include "lx200fs2.h"
#include "lx200gemini.h"
#include "lx200pulsar2.h"
#include "lx200ss2000pc.h"
#include "lx200zeq25.h"
#include "lx200gotonova.h"
#include "ioptronHC8406.h"
#include "lx200am5.h"
#include "lx200_pegasus_nyx101.h"
#include <cmath>
#include <memory>
#include <cstring>
#include <unistd.h>
#include "eq500x.h"

static class Loader
{
        std::unique_ptr<LX200Generic> telescope;
    public:
        Loader()
        {
            // Note: these if statements use strstr() which isn't a full string match, just a substring,
            // so if one driver name is the start of another's name, it needs to be AFTER the longer one!
#if defined lx200_driver_indi_lx200classic
            {
                IDLog("initializing from LX200 classic device...\n");
                telescope.reset(new LX200Classic());
            }
#elif defined lx200_driver_indi_lx200_OnStep
            {
                IDLog("initializing from LX200 OnStep device...\n");
                telescope.reset(new LX200_OnStep());
            }
#elif defined lx200_driver_indi_lx200gps
            {
                IDLog("initializing from LX200 GPS device...\n");
                telescope.reset(new LX200GPS());
            }
#elif defined lx200_driver_indi_lx200_16
            {
                IDLog("Initializing from LX200 16 device...\n");
                telescope.reset(new LX200_16());
            }
#elif defined lx200_driver_indi_lx200autostar
            {
                IDLog("initializing from Autostar device...\n");
                telescope.reset(new LX200Autostar());
            }
#elif defined lx200_driver_indi_lx200ap_v2
            {
                IDLog("initializing from Astrophysics V2 device...\n");
                telescope.reset(new LX200AstroPhysicsV2());
            }
#elif defined lx200_driver_indi_lx200gemini
            {
                IDLog("initializing from Losmandy Gemini device...\n");
                telescope.reset(new LX200Gemini());
            }
#elif defined lx200_driver_indi_lx200zeq25
            {
                IDLog("initializing from ZEQ25 device...\n");
                telescope.reset(new LX200ZEQ25());
            }
#elif defined lx200_driver_indi_lx200gotonova
            {
                IDLog("initializing from GotoNova device...\n");
                telescope.reset(new LX200GotoNova());
            }
#elif defined lx200_driver_indi_ioptronHC8406
            {
                IDLog("initializing from ioptron telescope Hand Controller HC8406 device...\n");
                telescope.reset(new ioptronHC8406());
            }
#elif defined lx200_driver_indi_lx200pulsar2
            {
                IDLog("initializing from pulsar2 device...\n");
                telescope.reset(new LX200Pulsar2());
            }
#elif defined lx200_driver_indi_lx200ss2000pc
            {
                IDLog("initializing from skysensor2000pc device...\n");
                telescope.reset(new LX200SS2000PC());
            }
#elif defined lx200_driver_indi_lx200fs2
            {
                IDLog("initializing from Astro-Electronic FS-2...\n");
                telescope.reset(new LX200FS2());
            }
#elif defined lx200_driver_indi_lx200_10micron
            {
                IDLog("initializing for 10Micron mount...\n");
                telescope.reset(new LX200_10MICRON());
            }
#elif defined lx200_driver_indi_eq500x
            {
                IDLog("initializing for EQ500X mount...\n");
                telescope.reset(new EQ500X());
            }
#elif defined lx200_driver_indi_lx200am5
            {
                IDLog("initializing for ZWO AM5 mount...\n");
                telescope.reset(new LX200AM5());
            }
#elif defined lx200_driver_indi_lx200_OpenAstroTech
            {
                IDLog("initializing for OpenAstroTech mount...\n");
                telescope.reset(new LX200_OpenAstroTech());
            }
#elif defined lx200_driver_indi_lx200_pegasus_nyx101
            {
                IDLog("initializing for Pegasus NYX-101 mount...\n");
                telescope.reset(new LX200NYX101());
            }
#elif defined lx200_driver_indi_lx200generic
            {
                telescope.reset(new LX200Generic());
            }
#else
#error "Driver name is not defined"            
#endif            
        }
} loader;

