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

HIDE := @

LOCAL_PATH := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
ifeq (true, $(APP_DEBUG))
	BIN_DIR := $(LOCAL_PATH)/bin/debug
	OBJ_DIR := $(LOCAL_PATH)/obj/debug
else
	BIN_DIR := $(LOCAL_PATH)/bin/release
	OBJ_DIR := $(LOCAL_PATH)/obj/release
endif
SOURCE_DIR := $(LOCAL_PATH)/../source
THIRD_PARTY_DIR := $(LOCAL_PATH)/../thirdparty

CC := clang++
AR := ar

C_FLAGS := 
C_FLAGS += -Wall 
C_FLAGS += -Werror=return-type
C_FLAGS += -fPIC
C_FLAGS += -pthread
ifeq (true, $(APP_DEBUG))
	C_FLAGS += -g -O0 -UNDEBUG
else
	C_FLAGS += -O2 -DNDEBUG
endif
C_FLAGS += -fvisibility=hidden
C_FLAGS += -DPAL_STDCPP_COMPAT=1
C_FLAGS += -I$(THIRD_PARTY_DIR)/CoreRT/src/Native/inc/unix
C_FLAGS += -I$(THIRD_PARTY_DIR)/DirectXMath/Inc
C_FLAGS += -std=c++17
C_FLAGS += -mbmi2

AR_FLAGS := 
AR_FLAGS += crsD

all :  \
	$(BIN_DIR)/libImportAsset.a

# Archive
$(BIN_DIR)/libImportAsset.a: \
	$(OBJ_DIR)/ImportAsset-brx_asset_import_file_input_stream.o \
	$(OBJ_DIR)/ImportAsset-brx_asset_import_memory_input_stream.o \
	$(OBJ_DIR)/ImportAsset-import_dds_image_asset.o \
	$(OBJ_DIR)/ImportAsset-import_pvr_image_asset.o \
	$(OBJ_DIR)/ImportAsset-import_gltf_scene_asset.o \
	$(OBJ_DIR)/ImportAsset-import_gltf_scene_asset_cgltf.o \
	$(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshNormals.o \
	$(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshTangentFrame.o \
	$(OBJ_DIR)/ImportAsset-thirdparty-McRT-Malloc-mcrt_malloc.o
	$(HIDE) mkdir -p $(BIN_DIR)
	$(HIDE) $(AR) $(AR_FLAGS) \
		$(BIN_DIR)/libImportAsset.a \
		$(OBJ_DIR)/ImportAsset-brx_asset_import_file_input_stream.o \
		$(OBJ_DIR)/ImportAsset-brx_asset_import_memory_input_stream.o \
		$(OBJ_DIR)/ImportAsset-import_dds_image_asset.o \
		$(OBJ_DIR)/ImportAsset-import_pvr_image_asset.o \
		$(OBJ_DIR)/ImportAsset-import_gltf_scene_asset.o \
		$(OBJ_DIR)/ImportAsset-import_gltf_scene_asset_cgltf.o \
		$(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshNormals.o \
		$(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshTangentFrame.o \
		$(OBJ_DIR)/ImportAsset-thirdparty-McRT-Malloc-mcrt_malloc.o

# Compile
$(OBJ_DIR)/ImportAsset-brx_asset_import_file_input_stream.o: $(SOURCE_DIR)/brx_asset_import_file_input_stream.cpp
	$(HIDE) mkdir -p $(OBJ_DIR)
	$(HIDE) $(CC) -c $(C_FLAGS) $(SOURCE_DIR)/brx_asset_import_file_input_stream.cpp -MD -MF $(OBJ_DIR)/ImportAsset-brx_asset_import_file_input_stream.d -o $(OBJ_DIR)/ImportAsset-brx_asset_import_file_input_stream.o

$(OBJ_DIR)/ImportAsset-brx_asset_import_memory_input_stream.o: $(SOURCE_DIR)/brx_asset_import_memory_input_stream.cpp
	$(HIDE) mkdir -p $(OBJ_DIR)
	$(HIDE) $(CC) -c $(C_FLAGS) $(SOURCE_DIR)/brx_asset_import_memory_input_stream.cpp -MD -MF $(OBJ_DIR)/ImportAsset-brx_asset_import_memory_input_stream.d -o $(OBJ_DIR)/ImportAsset-brx_asset_import_memory_input_stream.o

$(OBJ_DIR)/ImportAsset-import_dds_image_asset.o: $(SOURCE_DIR)/import_dds_image_asset.cpp
	$(HIDE) mkdir -p $(OBJ_DIR)
	$(HIDE) $(CC) -c $(C_FLAGS) $(SOURCE_DIR)/import_dds_image_asset.cpp -MD -MF $(OBJ_DIR)/ImportAsset-import_dds_image_asset.d -o $(OBJ_DIR)/ImportAsset-import_dds_image_asset.o

$(OBJ_DIR)/ImportAsset-import_pvr_image_asset.o: $(SOURCE_DIR)/import_pvr_image_asset.cpp
	$(HIDE) mkdir -p $(OBJ_DIR)
	$(HIDE) $(CC) -c $(C_FLAGS) $(SOURCE_DIR)/import_pvr_image_asset.cpp -MD -MF $(OBJ_DIR)/ImportAsset-import_pvr_image_asset.d -o $(OBJ_DIR)/ImportAsset-import_pvr_image_asset.o

$(OBJ_DIR)/ImportAsset-import_gltf_scene_asset.o: $(SOURCE_DIR)/import_gltf_scene_asset.cpp
	$(HIDE) mkdir -p $(OBJ_DIR)
	$(HIDE) $(CC) -c $(C_FLAGS) $(SOURCE_DIR)/import_gltf_scene_asset.cpp -MD -MF $(OBJ_DIR)/ImportAsset-import_gltf_scene_asset.d -o $(OBJ_DIR)/ImportAsset-import_gltf_scene_asset.o

$(OBJ_DIR)/ImportAsset-import_gltf_scene_asset_cgltf.o: $(SOURCE_DIR)/import_gltf_scene_asset_cgltf.cpp
	$(HIDE) mkdir -p $(OBJ_DIR)
	$(HIDE) $(CC) -c $(C_FLAGS) $(SOURCE_DIR)/import_gltf_scene_asset_cgltf.cpp -MD -MF $(OBJ_DIR)/ImportAsset-import_gltf_scene_asset_cgltf.d -o $(OBJ_DIR)/ImportAsset-import_gltf_scene_asset_cgltf.o

$(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshNormals.o: $(THIRD_PARTY_DIR)/DirectXMesh/DirectXMesh/DirectXMeshNormals.cpp
	$(HIDE) mkdir -p $(OBJ_DIR)
	$(HIDE) $(CC) -c $(C_FLAGS) $(THIRD_PARTY_DIR)/DirectXMesh/DirectXMesh/DirectXMeshNormals.cpp -MD -MF $(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshNormals.d -o $(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshNormals.o

$(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshTangentFrame.o: $(THIRD_PARTY_DIR)/DirectXMesh/DirectXMesh/DirectXMeshTangentFrame.cpp
	$(HIDE) mkdir -p $(OBJ_DIR)
	$(HIDE) $(CC) -c $(C_FLAGS) $(THIRD_PARTY_DIR)/DirectXMesh/DirectXMesh/DirectXMeshTangentFrame.cpp -MD -MF $(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshTangentFrame.d -o $(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshTangentFrame.o

$(OBJ_DIR)/ImportAsset-thirdparty-McRT-Malloc-mcrt_malloc.o: $(THIRD_PARTY_DIR)/McRT-Malloc/source/mcrt_malloc.cpp
	$(HIDE) mkdir -p $(OBJ_DIR)
	$(HIDE) $(CC) -c $(C_FLAGS) $(THIRD_PARTY_DIR)/McRT-Malloc/source/mcrt_malloc.cpp -MD -MF $(OBJ_DIR)/ImportAsset-thirdparty-McRT-Malloc-mcrt_malloc.d -o $(OBJ_DIR)/ImportAsset-thirdparty-McRT-Malloc-mcrt_malloc.o

-include \
	$(OBJ_DIR)/ImportAsset-brx_asset_import_file_input_stream.d \
	$(OBJ_DIR)/ImportAsset-brx_asset_import_memory_input_stream.d \
	$(OBJ_DIR)/ImportAsset-import_dds_image_asset.d \
	$(OBJ_DIR)/ImportAsset-import_pvr_image_asset.d \
	$(OBJ_DIR)/ImportAsset-import_gltf_scene_asset.d \
	$(OBJ_DIR)/ImportAsset-import_gltf_scene_asset_cgltf.d \
	$(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshNormals.d \
	$(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshTangentFrame.d \
	$(OBJ_DIR)/ImportAsset-thirdparty-McRT-Malloc-mcrt_malloc.d

clean:
	$(HIDE) rm -f $(BIN_DIR)/libImportAsset.a
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-brx_asset_import_file_input_stream.o
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-brx_asset_import_memory_input_stream.o
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-import_dds_image_asset.o
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-import_pvr_image_asset.o
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-import_gltf_scene_asset_cgltf.o
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-import_gltf_scene_asset.o
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshNormals.o
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshTangentFrame.o
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-thirdparty-McRT-Malloc-mcrt_malloc.o
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-brx_asset_import_file_input_stream.d
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-brx_asset_import_memory_input_stream.d
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-import_dds_image_asset.d
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-import_pvr_image_asset.d
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-import_gltf_scene_asset_cgltf.d
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-import_gltf_scene_asset.d
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshNormals.d
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-thirdparty-DirectXMesh-DirectXMeshTangentFrame.d
	$(HIDE) rm -f $(OBJ_DIR)/ImportAsset-thirdparty-McRT-Malloc-mcrt_malloc.d

.PHONY : \
	all \
	clean