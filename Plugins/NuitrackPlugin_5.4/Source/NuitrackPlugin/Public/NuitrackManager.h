#pragma once

#include "nuitrack/Nuitrack.h"

struct Quat { float i, j, k, l; };

int NuitrackManager_GetDevicesCount_impl();
const char* NuitrackManager_GetInstancesJson_impl();

const char* NuitrackManager_GetErrorMsg();
bool NuitrackManager_IsRunning();
bool NuitrackManager_Start_impl(bool AIMode, bool mirror, bool depthToColorReg, int sensorRotation, const char* bytes);
bool NuitrackManager_Update_impl();
void NuitrackManager_Stop_impl();

tdv::nuitrack::Vector3 NuitrackManager_RealToLocation_impl(tdv::nuitrack::Joint joint);
tdv::nuitrack::Vector3 NuitrackManager_ProjToLocation_impl(tdv::nuitrack::Joint joint);
Quat NuitrackManager_RotationMatrixTQuat_impl(tdv::nuitrack::Joint joint);

void NuitrackManager_OnNewGesture_impl(tdv::nuitrack::GestureData::Ptr gesturePtr);

int NuitrackManager_GetSkeletonsCount_impl();
tdv::nuitrack::SkeletonData::Ptr NuitrackManager_GetSkeleton_impl(int id);

tdv::nuitrack::ColorSensor::Ptr NuitrackManager_GetColorSensor_impl();
tdv::nuitrack::DepthSensor::Ptr NuitrackManager_GetDepthSensor_impl();

tdv::nuitrack::UserFrame::Ptr NuitrackManager_GetUserFrame_impl();
tdv::nuitrack::GestureData::Ptr NuitrackManager_GetGestureData_impl();
tdv::nuitrack::HandTrackerData::Ptr NuitrackManager_GetHandTrackerData_impl();