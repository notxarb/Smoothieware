/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FIRMWARESCREEN_H
#define FIRMWARESCREEN_H

#include "PanelScreen.h"

#include <string>

class FirmwareScreen : public PanelScreen {
    public:
        FirmwareScreen();
        void on_enter();
        void on_exit();
        void on_refresh();
        void on_main_loop();
        void clicked_line(uint16_t line);
        void display_menu_line(uint16_t line);

    private:
        void enter_folder(const char *folder);
        uint16_t count_folder_content();
        std::string file_at(uint16_t line, bool& isdir);
        bool filter_file(const char *f);
        bool done_copy;
        size_t copied_bytes;
        bool copying;
};

#endif
