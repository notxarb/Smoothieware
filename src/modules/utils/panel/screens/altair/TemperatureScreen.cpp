/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "libs/Kernel.h"
#include "Panel.h"
#include "PanelScreen.h"
#include "LcdBase.h"
#include "TemperatureScreen.h"
#include "libs/nuts_bolts.h"
#include "libs/utils.h"
#include "modules/utils/player/PlayerPublicAccess.h"
#include "modules/utils/player/Player.h"
#include "PublicDataRequest.h"
#include "PublicData.h"
#include "checksumm.h"
#include "ModifyValuesScreen.h"
#include "Robot.h"
#include "Planner.h"
#include "StepperMotor.h"
#include "EndstopsPublicAccess.h"
#include "TemperatureControlPublicAccess.h"
#include "TemperatureControlPool.h"

#include <string>
using namespace std;

static float getTargetTemperature(uint16_t heater_cs)
{
    struct pad_temperature temp;
    bool ok = PublicData::get_value( temperature_control_checksum, current_temperature_checksum, heater_cs, &temp );

    if (ok) {
        return temp.target_temperature;
    }

    return 0.0F;
}

TemperatureScreen::TemperatureScreen()
{
    ModifyValuesScreen(false);

    int cnt= 0;
    // returns enabled temperature controllers
    std::vector<struct pad_temperature> controllers;
    bool ok = PublicData::get_value(temperature_control_checksum, poll_controls_checksum, &controllers);
    if (ok) {
        for (auto &c : controllers) {
            // rename if two of the known types
            const char *name;
            if(c.designator == "T") name= "Hotend";
            else if(c.designator == "B") name= "Bed";
            else name= c.designator.c_str();
            uint16_t i= c.id;

            this->addMenuItem(name, // menu name
                [i]() -> float { return getTargetTemperature(i); }, // getter
                [i](float t) { PublicData::set_value( temperature_control_checksum, i, &t ); }, // setter
                1.0F, // increment
                0.0F, // Min
                500.0F // Max
            );
            cnt++;
        }
    }
}

TemperatureScreen::~TemperatureScreen()
{
    this->~ModifyValuesScreen();
}

void TemperatureScreen::on_refresh()
{
    ((ModifyValuesScreen *)this)->on_refresh();
}

void TemperatureScreen::on_enter()
{
    ((ModifyValuesScreen *)this)->on_enter();
}

void TemperatureScreen::display_menu_line(unsigned short line)
{
    ((ModifyValuesScreen *)this)->display_menu_line(line);
}

void TemperatureScreen::clicked_menu_entry(unsigned short line)
{
    ((ModifyValuesScreen *)this)->clicked_menu_entry(line);
}
