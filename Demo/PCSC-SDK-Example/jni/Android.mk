LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE	:=	PCSC
LOCAL_SRC_FILES	:=	PCSC.c
LOCAL_CFLAGS	:=	-I./depend -I./depend/PCSC -fno-short-enums
LOCAL_LDLIBS	:=	-L./depend -lpcsclite32_0.4 -llog

include $(BUILD_SHARED_LIBRARY)