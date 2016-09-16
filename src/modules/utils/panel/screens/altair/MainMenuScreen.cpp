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
#include "MainMenuScreen.h"
#include "WatchScreen.h"
#include "FileScreen.h"
#include "AdvancedScreen.h"
#include "BasicScreen.h"
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

#define extruder_checksum CHECKSUM("extruder")

MainMenuScreen::MainMenuScreen()
{
    // Children screens
    this->basic_screen    = (new BasicScreen()    )->set_parent(this);
    this->advanced_screen = (new AdvancedScreen() )->set_parent(this);
    this->file_screen     = (new FileScreen()     )->set_parent(this);
    this->watch_screen    = (new WatchScreen()    )->set_parent(this);
    this->set_parent(this->watch_screen);
}

void MainMenuScreen::on_enter()
{
    THEPANEL->enter_menu_mode();
    THEPANEL->setup_menu(THEPANEL->is_playing()?5:4);
    this->refresh_menu();
}

void MainMenuScreen::on_refresh()
{
    if ( THEPANEL->menu_change() ) {
        this->refresh_menu();
    }
    if ( THEPANEL->click() ) {
        this->clicked_menu_entry(THEPANEL->get_menu_current_line());
    }
}

void MainMenuScreen::display_menu_line(uint16_t line)
{
    switch(line) {
        case 0: THEPANEL->lcd->printf("Watch"); break;
        case 1:
            if(THEKERNEL->is_halted()) {
                THEPANEL->lcd->printf("Clear HALT");
            }
            else {
                if ( ! THEPANEL->is_playing() ) {
                    THEPANEL->lcd->printf("Print");
                }
                else {
                    if ( THEPANEL->is_suspended() ) {
                        THEPANEL->lcd->printf("Resume");
                    }
                    else {
                        THEPANEL->lcd->printf("Pause");
                    }
                }
            }
            break; // pause or resume
        case 2: THEPANEL->lcd->printf("Basic"); break;
        case 3: THEPANEL->lcd->printf("Advanced"); break;
        case 4: THEPANEL->lcd->printf("Abort"); break;
    }
}

void MainMenuScreen::clicked_menu_entry(uint16_t line)
{
    switch(line) {
        case 0: THEPANEL->enter_screen(this->watch_screen); break;
        case 1:
            if(THEKERNEL->is_halted()) {
                send_command("M999");
                THEPANEL->enter_screen(this->watch_screen);
            }
            else {
                if ( ! THEPANEL->is_playing() ) {
                    THEPANEL->enter_screen(this->file_screen);
                }
                else {
                    if ( THEPANEL->is_suspended() ) {
                        send_command("M601");
                        THEPANEL->enter_screen(this->watch_screen);
                    }
                    else {
                        THEPANEL->lcd->printf("Pause");
                        send_command("M600");
                        THEPANEL->enter_screen(this->watch_screen);
                    }
                }
            }
            break;
        case 2: THEPANEL->enter_screen(this->basic_screen); break;
        case 3: THEPANEL->enter_screen(this->advanced_screen); break;
        case 4: abort_playing(); break;
    }
}

void MainMenuScreen::abort_playing()
{
    //PublicData::set_value(player_checksum, abort_play_checksum, NULL);
    send_command("abort");
    THEPANEL->enter_screen(this->watch_screen);
}

