#pragma once

#define IS_DEV 1
#define IS_BETA 1  // Enable to deliver beta-exclusive gear via DeliveryBox
#ifndef GIT_SHA
#define GIT_SHA "unknown"
#endif
#define DEV_VER GIT_SHA
#define MOD_VER "2.0.0"