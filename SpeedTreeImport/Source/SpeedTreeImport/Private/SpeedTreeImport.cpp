// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "SpeedTreeImportPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "SpeedTreeImportStyle.h"
#include "SpeedTreeImportCommands.h"

#include "LevelEditor.h"
#include "EngineUtils.h"

static const FName SpeedTreeImportTabName("SpeedTreeImport");

#define LOCTEXT_NAMESPACE "FSpeedTreeImportModule";

void FSpeedTreeImportModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FSpeedTreeImportStyle::Initialize();
	FSpeedTreeImportStyle::ReloadTextures();

	FSpeedTreeImportCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSpeedTreeImportCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FSpeedTreeImportModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FSpeedTreeImportModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FSpeedTreeImportModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FSpeedTreeImportModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FSpeedTreeImportStyle::Shutdown();

	FSpeedTreeImportCommands::Unregister();
}

bool FSpeedTreeImportModule::isSpeedTree(AActor* TargetActor, bool bNested)
{
	FString _Folder = (TargetActor->GetFolderPath()).ToString();

	TArray<FString> Parsed;

	_Folder.ParseIntoArray(Parsed, TEXT("/"), false);

	if (bNested) return Parsed[0] == "SpeedTree" && Parsed.Num() > 1;
	return Parsed[0] == "SpeedTree" && Parsed.Num() == 1;
}

AActor* FSpeedTreeImportModule::CreateCloneOfMyActor(AActor* ExistingActor, FVector SpawnLocation, FRotator SpawnRotation)
{
	UWorld* World = ExistingActor->GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Template = ExistingActor;
	AActor* actor = World->SpawnActor<AActor>(ExistingActor->GetClass(), SpawnLocation, SpawnRotation, SpawnParams);
	
	return actor;
}

AActor* FSpeedTreeImportModule::FindActorInSelected(FString ActorName)
{
	AActor* TargetActor = NULL;
			
	for (FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
	{
		TargetActor = static_cast<AActor*>(*It);
			
		FString TreeName = TargetActor->GetName();
		
		if (TreeName.Contains(ActorName) && isSpeedTree(TargetActor, false))
		{
			return TargetActor;
		}
	}
	
	return NULL;
}


void FSpeedTreeImportModule::deleteActors(FString ActorName, AActor* mainActor)
{
	AActor* TargetActor = NULL;

	for (TActorIterator<AStaticMeshActor> It(GEditor->GetEditorWorldContext().World()); It; ++It)
	{
		TargetActor = static_cast<AActor*>(*It);

		FString TreeName = TargetActor->GetName();
		
		if (TreeName.Contains(ActorName) && isSpeedTree(TargetActor, true))
		{
			//GEditor->GetEditorWorldContext().World()->DestroyActor(TargetActor);
			TargetActor->Destroy();
		}
	}
}


void FSpeedTreeImportModule::PluginButtonClicked()
{
	FText Title1 = FText::FromString(TEXT("Confirm action"));
	FText DialogText1 = FText::FromString(TEXT("Do you really want to import trees?"));	
	if (FMessageDialog::Open(EAppMsgType::YesNo, DialogText1, &Title1) == EAppReturnType::No) return;
	
	FString _fname = "C:/temp/trees.ustf";

	TArray<FString> _secnames;
	TArray<FString> _data;
	TArray<FString> _tmp;
	
	// Get all sections names
	GConfig->GetSectionNames(*_fname, _secnames);
	
	FString _tmpString;
	
	// Parse all data in section
	int i = 0;
	for (auto& s : _secnames)
	{
		GConfig->GetSection(*s, _tmp, *_fname);
				
		AActor* _foundTree = FindActorInSelected(*s);
		
		if (_foundTree == nullptr) continue;

		deleteActors(*s, _foundTree);
	
		for (auto& v : _tmp)
		{
			TArray<FString> _transform;
			TArray<FString> _translate;

			v.ParseIntoArray(_data, TEXT("="), false);
			_data[1].ParseIntoArray(_transform, TEXT("@"), false);
			_transform[0].ParseIntoArray(_translate, TEXT(" "), false);
			
			AActor* _newTree = CreateCloneOfMyActor(_foundTree, FVector::ZeroVector, FRotator::ZeroRotator);
			_newTree->SetActorLabel(_data[0]);
			
			_newTree->SetFolderPath(FName(*FString::Printf(TEXT("SpeedTree/%s"), *s)));
			
		
			// Set scale 
			float _scaleZ = FCString::Atof(*_transform[2]);
					
			FTransform _uTransform;
			_uTransform.SetIdentity();
			_uTransform.SetRotation(FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(FCString::Atof(*_transform[1]))));
			_uTransform.SetLocation(FVector(FCString::Atod(*_translate[0]), FCString::Atod(*_translate[1]), FCString::Atod(*_translate[2])));
			_uTransform.SetScale3D(FVector(_scaleZ, _scaleZ, _scaleZ));

			i++;

			_newTree->SetActorTransform(_uTransform);
			

			//_tmpString += _translate[0] + TEXT(" | ") + _translate[1] + TEXT(" | ") + _translate[2];
			//_tmpString += TEXT(", \n");
		}	
	}

	//UE_LOG(LogTemp, Warning, TEXT("%s"), _tmpString);

	GConfig->Flush(true, *_fname);
	
	if (i > 0)
	{
		FText Title2 = FText::FromString(TEXT("Import Success!"));
		FString DialogText2 = FString::Printf(TEXT("Imported %d tree%s"), i, (i < 2) ? "" : "s");
		FMessageDialog::Debugf(FText::FromString(DialogText2), &Title2);
	}
	else
	{
		FText Title3 = FText::FromString(TEXT("Import Failed!"));
		FString DialogText3 = FString::Printf(TEXT("Please select at least %d main tree\nfrom \"SpeedTree\" folder!"), 1);
		FMessageDialog::Debugf(FText::FromString(DialogText3), &Title3);
	}

	//// Foliage
	//
	//TActorIterator<AInstancedFoliageActor> foliageIterator(GEditor->GetEditorWorldContext().World()); //GetWorld()
	//AInstancedFoliageActor* foliageActor = *foliageIterator;

	//TArray<UInstancedStaticMeshComponent*> components;
	//foliageActor->;
	//	
	//
	//UInstancedStaticMeshComponent* meshComponent = components[0];

	//FTransform transform = FTransform();
	//for (int32 x = 1; x < 20; x++)
	//{
	//	for (int32 y = 1; y < 20; y++)
	//	{
	//		transform.SetLocation(FVector(1000.f * x, 1000.f * y, 0.f));
	//		meshComponent->AddInstance(transform);
	//	}
	//}
	//

	/*
	for (TActorIterator<AInstancedFoliageActor> ActorItr(GEditor->GetEditorWorldContext().World()); ActorItr; ++ActorItr)
	{
		// Same as with the Objmect Iterator, access the subclass instance with the * or -> operators.
		AInstancedFoliageActor *foliageActor = *ActorItr;
		
		//TArray<UInstancedStaticMeshComponent*> components;
		//GetComponents<UInstancedStaticMeshComponent>(components);


		//UE_LOG(LogTemp, Warning, TEXT("%s"), ActorItr->GetName());

		//ClientMessage(ActorItr->GetActorLocation().ToString());
	}*/

	/* !! Work
	AActor* TargetActor = NULL;
	AInstancedFoliageActor* CurrentIFA = NULL;
	
	FString TargetName;
	FString TargetLocation;
	

	for (FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
	{
		TargetActor = static_cast<AActor*>(*It);
		//CurrentIFA = static_cast<AInstancedFoliageActor*>(*It);
		//CurrentIFA = (AInstancedFoliageActor*)TargetActor;
		//CurrentIFA = Cast<AInstancedFoliageActor*>(*It);

		if (TargetActor)
		{
			TargetName = TargetActor->GetName();
			TargetLocation = TargetActor->GetActorLocation().ToString();

			FVector TreePos = FVector (0.f, 0.f, 0.f);
							
			UWorld* World = TargetActor->GetWorld();
			FActorSpawnParameters SpawnParams;
			SpawnParams.Template = TargetActor;
			AActor* actor = World->SpawnActor<AActor>(TargetActor->GetClass(), TreePos, FRotator::ZeroRotator, SpawnParams);

			UE_LOG(LogTemp, Warning, TEXT("Name: %s, Location: %s"), *(TargetName), *(TargetLocation));
		}
	}*/

	/*
	for (TActorIterator<AStaticMeshActor> ActorItr(GEditor->GetEditorWorldContext().World()); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		AStaticMeshActor *Mesh = *ActorItr;
		
		UE_LOG(LogTemp, Warning, TEXT("%s"), *ActorItr->GetName());

		//ClientMessage(ActorItr->GetActorLocation().ToString());
	}*/


	

	/*
	UObject* ObjectToSpawn;
	ObjectToSpawn = FindObject<UObject>(ANY_PACKAGE, TEXT("/Game/SpeedTree/SimpleActor"));
	
	if (ObjectToSpawn)
	{
		UClass * ClassToSpawn = ObjectToSpawn->StaticClass();

		if (ClassToSpawn)
		{
			const FVector Location = FVector(0, 0, 0);
			const FRotator Rotation = FRotator(0, 0, 0);
			AActor * _newTree = GWorld->SpawnActor(ClassToSpawn, &Location, &Rotation);
			
			//UE_LOG(LogTemp, Warning, TEXT("%s"), *_newTree->GetName());
		}
	}
	*/

	/*
	UWorld* const World = GEditor->GetEditorWorldContext().World();
	//FVector* SpawnLocation = FVector(-30.f, -1300.f, 300.f);

	AProceduralFoliageVolume* Cube = Cast<AProceduralFoliageVolume>(GEditor->AddActor(World->GetCurrentLevel(), AProceduralFoliageVolume::StaticClass(), FTransform(FVector(-30.f, -1300.f, 300.f))));

	UCubeBuilder* Builder = NewObject<UCubeBuilder>(UCubeBuilder::StaticClass());
	Cube->BrushBuilder = Builder;
	Cube->Brush = Cast<UModel>(World->GetDefaultBrush());
	Builder->X = 200.f;
	Builder->Y = 200.f;
	Builder->Z = 300.f;
	Builder->PostEditChange();
	Builder->Build(World, World->GetDefaultBrush());
	*/
	
	
	/*
	AProceduralFoliageVolume* ClonedVolume = Cast<AProceduralFoliageVolume>(GEditor->AddActor(World->GetCurrentLevel(), AProceduralFoliageVolume::StaticClass(), FTransform(CloneLocation)));

	ClonedVolume->BrushComponent->Brush = OriginalVolume->BrushComponent->Brush;
	ClonedVolume->BrushComponent->BrushBodySetup = OriginalVolume->BrushComponent->BrushBodySetup;

	ClonedVolume->ProceduralComponent->FoliageSpawner = OriginalVolume->ProceduralComponent->FoliageSpawner;
	ClonedVolume->ProceduralComponent->TileOverlap = OriginalVolume->ProceduralComponent->TileOverlap;
	ClonedVolume->ProceduralComponent->bShowDebugTiles = OriginalVolume->ProceduralComponent->bShowDebugTiles;

	ClonedVolume->Brush = OriginalVolume->Brush;
	ClonedVolume->BrushBuilder = OriginalVolume->BrushBuilder;
	ClonedVolume->BrushColor = OriginalVolume->BrushColor;
	ClonedVolume->BrushType = OriginalVolume->BrushType;

	ClonedVolume->BrushBuilder->Build(World, Cast<ABrush>(ClonedVolume->Brush));
	ClonedVolume->MarkComponentsRenderStateDirty();
	*/

	
}

void FSpeedTreeImportModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FSpeedTreeImportCommands::Get().PluginAction);
}

void FSpeedTreeImportModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FSpeedTreeImportCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSpeedTreeImportModule, SpeedTreeImport)