//
// Copyright (C) YuqiaoZhang(HanetakaChou)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef _USER_INPUT_CONTROLLER_H_
#define _USER_INPUT_CONTROLLER_H_ 1

#include "user_input_model.h"
#include "../thirdparty/DXUT/Optional/DXUTcamera.h"
#include "../thirdparty/Brioche-Analytic-Rendering-Interface/include/brx_anari.h"
#include "../thirdparty/Brioche-Motion/include/brx_motion.h"

struct ui_controller_t
{
    int m_language_index;
    bool m_import_asset_image_force_srgb;
    int m_import_asset_image_get_open_file_name_file_type_index;
    mcrt_string m_selected_asset_image;
    int m_import_model_asset_get_open_file_name_file_type_index;
    int m_import_motion_asset_get_open_file_name_file_type_index;
};

struct user_camera_controller_t
{
    CDXUTFirstPersonCamera m_first_person_camera;
};

extern void ui_controller_init(ui_controller_t *ui_controller);

extern void ui_simulate(void *platform_context, brx_anari_device *device, ui_model_t *ui_model, ui_controller_t *ui_controller);

extern void user_camera_controller_init(brx_anari_device *device, user_camera_model_t const *user_camera_model, user_camera_controller_t *user_camera_controller);

extern void user_camera_simulate(float interval_time, brx_anari_device *device, user_camera_model_t *user_camera_model, user_camera_controller_t *user_camera_controller);

#endif