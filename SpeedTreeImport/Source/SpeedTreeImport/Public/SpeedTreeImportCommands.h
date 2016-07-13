// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "SpeedTreeImportStyle.h"

class FSpeedTreeImportCommands : public TCommands<FSpeedTreeImportCommands>
{
public:

	FSpeedTreeImportCommands()
		: TCommands<FSpeedTreeImportCommands>(TEXT("SpeedTreeImport"), NSLOCTEXT("Contexts", "SpeedTreeImport", "SpeedTreeImport Plugin"), NAME_None, FSpeedTreeImportStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
