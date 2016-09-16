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
#include "BasicScreen.h"
#include "WatchScreen.h"
#include "FileScreen.h"
#include "JogScreen.h"
#include "ControlScreen.h"
#include "PrepareScreen.h"
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

BasicScreen::BasicScreen()
{
    // Children screens
}

void BasicScreen::on_enter()
{
    THEPANEL->enter_menu_mode();
    THEPANEL->setup_menu(1);
    this->refresh_menu();
}

void BasicScreen::on_refresh()
{
    if ( THEPANEL->menu_change() ) {
        this->refresh_menu();
    }
    if ( THEPANEL->click() ) {
        this->clicked_menu_entry(THEPANEL->get_menu_current_line());
    }
}

void BasicScreen::display_menu_line(uint16_t line)
{
    switch(line) {
        case 1: THEPANEL->lcd->printf("Back"); break;
    }
}

void BasicScreen::clicked_menu_entry(uint16_t line)
{
    switch(line) {
        case 1: THEPANEL->enter_screen(this->parent); break;
    }
}
