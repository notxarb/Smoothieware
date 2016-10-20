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
#include "TemperatureScreen.h"
#include "ConfigureScreen.h"
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

#include <string>
using namespace std;

AdvancedScreen::AdvancedScreen()
{
    // Children screens
  this->extruder_screen    = (new ExtruderScreen()   )->set_parent(this);
  this->temperature_screen = (new TemperatureScreen())->set_parent(this);
  this->jog_screen         = (new JogScreen()        )->set_parent(this);
  this->configure_screen   = (new ConfigureScreen()  )->set_parent(this);
  this->probe_screen       = (new ProbeScreen()      )->set_parent(this);
}

void AdvancedScreen::on_enter()
{
    THEPANEL->enter_menu_mode();
    THEPANEL->setup_menu(13);
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
      case  1: THEPANEL->lcd->printf("Set Bed Height"); break;
      case  2: THEPANEL->lcd->printf("Motors Off"); break;
      case  3: THEPANEL->lcd->printf("Preheat"); break;
      case  4: THEPANEL->lcd->printf("Cool Down"); break;
      case  5: THEPANEL->lcd->printf("Extruder"); break;
      case  6: THEPANEL->lcd->printf("Set Temperature"); break;
      case  7: THEPANEL->lcd->printf("Hot End PID"); break;
      case  8: THEPANEL->lcd->printf("Bed PID"); break;
      case  9: THEPANEL->lcd->printf("Go To Z5"); break;
      case 10: THEPANEL->lcd->printf("Jog"); break;
      case 11: THEPANEL->lcd->printf("Configure"); break;
      case 12: THEPANEL->lcd->printf("Probe"); break;
    }
}

void AdvancedScreen::clicked_menu_entry(uint16_t line)
{
    switch(line) {
      case  0: THEPANEL->enter_screen(this->parent); break;
	  case  1: send_command("M306Z0");
		       send_command("M500");
		       break;
	  case  2: send_command("M84"); break;
	  case  3: //this->preheat(); break;
	  case  4: //this->cooldown(); break;
      case  5: THEPANEL->enter_screen(this->extruder_screen); break;
      case  6: THEPANEL->enter_screen(this->temperature_screen); break;
	  case  7: send_command("M117Tuning HotEnd");
		       send_command("M303 E0 S220");
		       send_command("M117PID Tuning Complete");
			   send_command("G4 S5;");
			   send_command("M117");
		  break;
	  case  8: send_command("M117Tuning Heated Bed");
		       send_command("M303 E1 S100");
		       send_command("M117PID Tuning Complete");
		       send_command("G4 S5;");
		       send_command("M117");
		       break;
	  case  9: send_command("G28");
		       send_command("G0Z5F3000"); 
			   break;
      case 10: THEPANEL->enter_screen(this->jog_screen); break;
      case 11: THEPANEL->enter_screen(this->configure_screen); break;
      case 12: THEPANEL->enter_screen(this->probe_screen); break;
    }
}
