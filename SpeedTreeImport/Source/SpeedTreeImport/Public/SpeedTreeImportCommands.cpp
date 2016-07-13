// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "SpeedTreeImportPrivatePCH.h"
#include "SpeedTreeImportCommands.h"

#define LOCTEXT_NAMESPACE "FSpeedTreeImportModule"

void FSpeedTreeImportCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "SpeedTreeImport", "Execute SpeedTreeImport action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
