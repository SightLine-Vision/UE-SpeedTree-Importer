// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FSpeedTreeImportModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
	bool isSpeedTree(AActor* TargetActor, bool bNested = false);
	AActor* CreateCloneOfMyActor(AActor* ExistingActor, FVector SpawnLocation, FRotator SpawnRotation);
	AActor* FindActorInSelected(FString ActorName);
	void deleteActors(FString ActorName, AActor* mainActor);	
	
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};