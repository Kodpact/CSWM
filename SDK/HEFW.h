#pragma once

void HookEntityFW(const char ClassName[], int Offset, void *Function, void *&Forward);
void HookEntityFWByVTable(void **VTable, int Offset, void *Function, void *&Forward);
void ResetEntityFW(const char ClassName[], int Offset, void *Function, void *Forward);
void ResetEntityFWByVTable(void **VTable, int Offset, void *Function, void *&Forward);
void *GetEntityFW(const char ClassName[], int Offset);

#include "CStrike.h"