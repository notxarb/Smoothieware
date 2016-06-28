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
#include "PrepareScreen.h"
#include "ExtruderScreen.h"
#include "libs/nuts_bolts.h"
#include "libs/utils.h"
#include "checksumm.h"
#include "PublicDataRequest.h"
#include "PublicData.h"
#include "TemperatureControlPublicAccess.h"
#include "ModifyValuesScreen.h"
#include "TemperatureControlPool.h"
#include "JogScreen.h"
#include "ProbeScreen.h"
#include "Robot.h"
#include "StepperMotor.h"
#include "Planner.h"
#include "EndstopsPublicAccess.h"

#include <string>
using namespace std;

PrepareScreen::PrepareScreen()
{
    // Children screens
    std::vector<struct pad_temperature> controllers;
    bool ok = PublicData::get_value(temperature_control_checksum, poll_controls_checksum, &controllers);
    if (ok && controllers.size() > 0) {
        this->extruder_screen = (new ExtruderScreen())->set_parent(this);
    }else{
        this->extruder_screen= nullptr;
    }
    this->jog_screen = (new JogScreen())->set_parent(this);
}

// setup and enter the configure screen
void PrepareScreen::setupConfigureScreen()
{
    auto mvs= new ModifyValuesScreen(true); // delete itself on exit
    mvs->set_parent(this);

    // acceleration
    mvs->addMenuItem("Acceleration", // menu name
        []() -> float { return THEKERNEL->planner->get_acceleration(); }, // getter
        [this](float acc) { send_gcode("M204", 'S', acc); }, // setter
        10.0F, // increment
        1.0F, // Min
        10000.0F // Max
        );

    // steps/mm
    mvs->addMenuItem("X steps/mm",
        []() -> float { return THEKERNEL->robot->actuators[0]->get_steps_per_mm(); },
        [](float v) { THEKERNEL->robot->actuators[0]->change_steps_per_mm(v); },
        0.1F,
        1.0F
        );

    mvs->addMenuItem("Y steps/mm",
        []() -> float { return THEKERNEL->robot->actuators[1]->get_steps_per_mm(); },
        [](float v) { THEKERNEL->robot->actuators[1]->change_steps_per_mm(v); },
        0.1F,
        1.0F
        );

    mvs->addMenuItem("Z steps/mm",
        []() -> float { return THEKERNEL->robot->actuators[2]->get_steps_per_mm(); },
        [](float v) { THEKERNEL->robot->actuators[2]->change_steps_per_mm(v); },
        0.1F,
        1.0F
        );

    mvs->addMenuItem("Z Home Ofs",
        []() -> float { void *rd; PublicData::get_value( endstops_checksum, home_offset_checksum, &rd ); return rd==nullptr ? 0.0F : ((float*)rd)[2]; },
        [this](float v) { send_gcode("M206", 'Z', v); },
        0.01F
        );

    mvs->addMenuItem("Contrast",
        []() -> float { return THEPANEL->lcd->getContrast(); },
        [this](float v) { THEPANEL->lcd->setContrast(v); },
        1,
        0,
        255,
        true // instant update
        );

    THEPANEL->enter_screen(mvs);
}

void PrepareScreen::on_enter()
{
    THEPANEL->enter_menu_mode();
    // if no heaters or extruder then don't show related menu items
    THEPANEL->setup_menu((this->extruder_screen != nullptr) ? 10 : 6);
    this->refresh_menu();
}

void PrepareScreen::on_refresh()
{
    if ( THEPANEL->menu_change() ) {
        this->refresh_menu();
    }
    if ( THEPANEL->click() ) {
        this->clicked_menu_entry(THEPANEL->get_menu_current_line());
    }
}

void PrepareScreen::display_menu_line(uint16_t line)
{
    switch ( line ) {
        case 0: THEPANEL->lcd->printf("Back"           ); break;
        case 1: THEPANEL->lcd->printf("Home All Axes"  ); break;
        case 2: THEPANEL->lcd->printf("Motors off"     ); break;
        case 3: THEPANEL->lcd->printf("Jog"            ); break;
        case 4: THEPANEL->lcd->printf("Configure"      ); break;
        case 5: THEPANEL->lcd->printf("Probe"          ); break;
        // these won't be accessed if no heaters or extruders
        case 6: THEPANEL->lcd->printf("Pre Heat"       ); break;
        case 7: THEPANEL->lcd->printf("Cool Down"      ); break;
        case 8: THEPANEL->lcd->printf("Extruder..."    ); break;
        case 9: THEPANEL->lcd->printf("Set Temperature"); break;
    }
}

void PrepareScreen::clicked_menu_entry(uint16_t line)
{
    switch ( line ) {
        case 0: THEPANEL->enter_screen(this->parent); break;
        case 1:
            send_command("G28");
            THEPANEL->enter_screen(this->parent);
            break;
        case 2:
            send_command("M84");
            THEPANEL->enter_screen(this->parent);
            break;
        case 3:
            THEPANEL->enter_screen(this->jog_screen     ); break;
        case 4:
            setupConfigureScreen(); break;
        case 5:
            THEPANEL->enter_screen((new ProbeScreen())->set_parent(this)); break;
        case 6:
            this->preheat();
            THEPANEL->enter_screen(this->parent);
            break;
        case 7:
            this->cooldown();
            THEPANEL->enter_screen(this->parent);
            break;
        case 8: THEPANEL->enter_screen(this->extruder_screen); break;
        case 9: setup_temperature_screen(); break;
    }
}

void PrepareScreen::preheat()
{
    float t = THEPANEL->get_default_hotend_temp();
    PublicData::set_value( temperature_control_checksum, hotend_checksum, &t );
    t = THEPANEL->get_default_bed_temp();
    PublicData::set_value( temperature_control_checksum, bed_checksum, &t );
}

void PrepareScreen::cooldown()
{
    float t = 0;
    std::vector<struct pad_temperature> controllers;
    bool ok = PublicData::get_value(temperature_control_checksum, poll_controls_checksum, &controllers);
    if (ok) {
        for (auto &c : controllers) {
            PublicData::set_value( temperature_control_checksum, c.id, &t );
        }
    }
 }

static float getTargetTemperature(uint16_t heater_cs)
{
    struct pad_temperature temp;
    bool ok = PublicData::get_value( temperature_control_checksum, current_temperature_checksum, heater_cs, &temp );

    if (ok) {
        return temp.target_temperature;
    }

    return 0.0F;
}

void PrepareScreen::setup_temperature_screen()
{
    // setup temperature screen
    auto mvs= new ModifyValuesScreen(true); // delete itself on exit
    mvs->set_parent(this);

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

            mvs->addMenuItem(name, // menu name
                [i]() -> float { return getTargetTemperature(i); }, // getter
                [i](float t) { PublicData::set_value( temperature_control_checksum, i, &t ); }, // setter
                1.0F, // increment
                0.0F, // Min
                500.0F // Max
            );
            cnt++;
        }
    }

    if(cnt > 0) {
        THEPANEL->enter_screen(mvs);
    }else{
        // no heaters and probably no extruders either
        delete mvs;
    }
}
