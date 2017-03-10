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
#include "AdvancedScreen.h"
#include "ExtruderScreen.h"
#include "JogScreen.h"
// #include "TemperatureScreen.h"
#include "ModifyValuesScreen.h"
// #include "ConfigureScreen.h"
#include "ProbeScreen.h"
#include "libs/nuts_bolts.h"
#include "libs/utils.h"
#include "modules/utils/player/PlayerPublicAccess.h"
#include "modules/utils/player/Player.h"
#include "PublicData.h"
#include "checksumm.h"
#include "ModifyValuesScreen.h"
#include "Robot.h"
#include "Planner.h"
#include "StepperMotor.h"
#include "EndstopsPublicAccess.h"
#include "ExtruderPublicAccess.h"
#include "TemperatureControlPublicAccess.h"
#include "TemperatureControlPool.h"

#include <string>
using namespace std;

#define extruder_checksum CHECKSUM("extruder")

static float getTargetTemperature(uint16_t heater_cs)
{
  struct pad_temperature temp;
  bool ok = PublicData::get_value( temperature_control_checksum, current_temperature_checksum, heater_cs, &temp );

  if (ok) {
    return temp.target_temperature;
  }

  return 0.0F;
}

AdvancedScreen::AdvancedScreen()
{
  // Children screens
  this->extruder_screen    = (new ExtruderScreen()   )->set_parent(this);
  // this->temperature_screen = (new TemperatureScreen())->set_parent(this);
  this->jog_screen         = (new JogScreen()        )->set_parent(this);
  // this->configure_screen   = (new ConfigureScreen()  )->set_parent(this);
  this->probe_screen       = (new ProbeScreen()      )->set_parent(this);

  // // Setup the temperature screen
  // this->temperature_screen = new ModifyValuesScreen(false);
  // this->temperature_screen->set_parent(this);

  // int cnt= 0;
  // // returns enabled temperature controllers
  // std::vector<struct pad_temperature> controllers;
  // bool ok = PublicData::get_value(temperature_control_checksum, poll_controls_checksum, &controllers);
  // if (ok) {
  //     for (auto &c : controllers) {
  //         // rename if two of the known types
  //         const char *name;
  //         if(c.designator == "T") name= "Hotend";
  //         else if(c.designator == "B") name= "Bed";
  //         else name= c.designator.c_str();
  //         uint16_t i= c.id;

  //         ((ModifyValuesScreen *)this->temperature_screen)->addMenuItem(name, // menu name
  //             [i]() -> float { return getTargetTemperature(i); }, // getter
  //             [i](float t) { PublicData::set_value( temperature_control_checksum, i, &t ); }, // setter
  //             1.0F, // increment
  //             0.0F, // Min
  //             500.0F // Max
  //         );
  //         cnt++;
  //     }
  // }

  // this->configure_screen = new ModifyValuesScreen(false);
  // this->configure_screen->set_parent(this);

  // ((ModifyValuesScreen *)this->configure_screen)->addMenuItem("Z Home Ofs",
  //   []() -> float { void *rd; PublicData::get_value( endstops_checksum, home_offset_checksum, &rd ); return rd==nullptr ? 0.0F : ((float*)rd)[2]; },
  //   [this](float v) { send_gcode("M206", 'Z', v); },
  //   0.01F
  //   );

  // ((ModifyValuesScreen *)this->configure_screen)->addMenuItem("Contrast",
  //   []() -> float { return THEPANEL->lcd->getContrast(); },
  //   [this](float v) { THEPANEL->lcd->setContrast(v); },
  //   1,
  //   0,
  //   255,
  //   true // instant update
  //   );

}

void AdvancedScreen::on_enter()
{
    THEPANEL->enter_menu_mode();
    THEPANEL->setup_menu(14);
    this->refresh_menu();
}

void AdvancedScreen::on_refresh()
{
    if ( THEPANEL->menu_change() ) {
        this->refresh_menu();
    }
    if ( THEPANEL->click() ) {
        this->clicked_menu_entry(THEPANEL->get_menu_current_line());
    }
}

void AdvancedScreen::display_menu_line(uint16_t line)
{
    switch(line) {
      case  0: THEPANEL->lcd->printf("Back"); break;
      case  1: THEPANEL->lcd->printf("Motors Off"); break;
      case  2: THEPANEL->lcd->printf("Preheat"); break;
      case  3: THEPANEL->lcd->printf("Cool Down"); break;
      case  4: THEPANEL->lcd->printf("Extruder"); break;
      case  5: THEPANEL->lcd->printf("Set Temperature"); break;
      case  6: THEPANEL->lcd->printf("Set Hot End PID"); break;
      case  7: THEPANEL->lcd->printf("Set Bed PID"); break;
      case  8: THEPANEL->lcd->printf("Go To Z5"); break;
      case  9: THEPANEL->lcd->printf("Jog"); break;
      case 10: THEPANEL->lcd->printf("Configure"); break;
      case 11: THEPANEL->lcd->printf("Probe"); break;
	    case 12: THEPANEL->lcd->printf("Set Bed Height"); break;
	    case 13: THEPANEL->lcd->printf("Save Settings"); break;
    }
}

void AdvancedScreen::clicked_menu_entry(uint16_t line)
{
    switch(line) {
      case  0: THEPANEL->enter_screen(this->parent); break;
      case  1: send_command("M84");
		       THEPANEL->enter_screen(this->parent);
		       break;
	    case  2: this->preheat(); 
		       THEPANEL->enter_screen(this->parent);
		       break;
      case  3: this->cooldown(); 
		       THEPANEL->enter_screen(this->parent);
		       break;
      case  4: THEPANEL->enter_screen(this->extruder_screen); break;
      // case  6: THEPANEL->enter_screen(this->temperature_screen); break;
      case  5: this->setupTemperatureSettings(); break;
      case  6: send_command("M303 E0 S220");
		       THEPANEL->enter_screen(this->parent);
               break;
      case  7: send_command("M303 E1 S100");
		       THEPANEL->enter_screen(this->parent);
               break;
      case  8: send_command("G28");
               send_command("G0Z5F3000"); 
			   THEPANEL->enter_screen(this->parent);
			   break;
      case  9: THEPANEL->enter_screen(this->jog_screen); break;
      case 10: this->setupConfigSettings(); break;
      // case 11: THEPANEL->enter_screen(this->configure_screen); break;
      case 11: ((ProbeScreen *)this->probe_screen)->set_watch_screen(this->watch_screen); THEPANEL->enter_screen(this->probe_screen); break;
      case 12: send_command("M306Z0");
           THEPANEL->enter_screen(this->parent);
           break;
	    case 13: send_command("M500");
		       THEPANEL->enter_screen(this->parent);
		       break;
    }
}

void AdvancedScreen::preheat()
{
    float t = THEPANEL->get_default_hotend_temp();
    PublicData::set_value( temperature_control_checksum, hotend_checksum, &t );
    t = THEPANEL->get_default_bed_temp();
    PublicData::set_value( temperature_control_checksum, bed_checksum, &t );
}

void AdvancedScreen::cooldown()
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

void AdvancedScreen::setupTemperatureSettings()
{
    // Setup the temperature screen
    auto mvs = new ModifyValuesScreen(true);
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

    THEPANEL->enter_screen(mvs);
}

void AdvancedScreen::setupConfigSettings()
{
    // This comes from the 3dprinter/MainMenuScreen.cpp
    auto mvs= new ModifyValuesScreen(true); // delete itself on exit
    mvs->set_parent(this);

    mvs->addMenuItem("Z Home Ofs",
        []() -> float { float rd[3]; PublicData::get_value( endstops_checksum, home_offset_checksum, rd ); return rd[2]; },
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
