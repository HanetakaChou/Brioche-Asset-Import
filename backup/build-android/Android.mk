#
# Copyright (C) YuqiaoZhang(HanetakaChou)
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

# https://developer.android.com/ndk/guides/android_mk

LOCAL_PATH := $(call my-dir)

# Import Asset

include $(CLEAR_VARS)

ifeq (armeabi-v7a, $(TARGET_ARCH_ABI))
LOCAL_ARM_MODE := arm
LOCAL_ARM_NEON := true
endif

LOCAL_MODULE := ImportAsset

LOCAL_SRC_FILES := \
	$(LOCAL_PATH)/../source/brx_asset_import_file_input_stream.cpp \
	$(LOCAL_PATH)/../source/brx_asset_import_memory_input_stream.cpp \
	$(LOCAL_PATH)/../source/import_dds_image_asset.cpp \
	$(LOCAL_PATH)/../source/import_pvr_image_asset.cpp \
	$(LOCAL_PATH)/../source/import_gltf_scene_asset.cpp \
	$(LOCAL_PATH)/../source/import_gltf_scene_asset_cgltf.cpp \
	$(LOCAL_PATH)/../thirdparty/DirectXMesh/DirectXMesh/DirectXMeshNormals.cpp \
	$(LOCAL_PATH)/../thirdparty/DirectXMesh/DirectXMesh/DirectXMeshTangentFrame.cpp \
	$(LOCAL_PATH)/../thirdparty/McRT-Malloc/source/mcrt_malloc.cpp

LOCAL_CFLAGS :=
LOCAL_CFLAGS += -Wall
LOCAL_CFLAGS += -Werror=return-type
LOCAL_CFLAGS += -fvisibility=hidden
ifeq (x86,$(TARGET_ARCH_ABI))
LOCAL_CFLAGS += -mbmi2
endif
ifeq (x86_64,$(TARGET_ARCH_ABI))
LOCAL_CFLAGS += -mbmi2
endif
LOCAL_CFLAGS += -DPAL_STDCPP_COMPAT=1

LOCAL_CPPFLAGS :=

LOCAL_C_INCLUDES :=
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../thirdparty/CoreRT/src/Native/inc/unix
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../thirdparty/DirectXMath/Inc

LOCAL_LDFLAGS :=

LOCAL_LDLIBS :=

include $(BUILD_STATIC_LIBRARY)
