// Fill out your copyright notice in the Description page of Project Settings.

#include "MyMassCrowdRepresentationActorManagement.h"
#include "MassActorSpawnerSubsystem.h"
#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassNPCCharacter.h"
#include "UGameEventSystem.h"
#include "StructUtils/StructView.h"

void UMyMassCrowdRepresentationActorManagement::OnPreActorSpawn(const FMassActorSpawnRequestHandle& SpawnRequestHandle, FConstStructView SpawnRequest, TSharedRef<FMassEntityManager> EntityManager) const
{
	if (EntityManager->GetWorld()->GetNetMode() != NM_Client)
	{
		Super::OnPreActorSpawn(SpawnRequestHandle, SpawnRequest, EntityManager);
		return;
	}
	
	UUGameEventSystem* EventSystem = EntityManager->GetWorld()->GetSubsystem<UUGameEventSystem>();
	if (EventSystem == nullptr)
	{
		Super::OnPreActorSpawn(SpawnRequestHandle, SpawnRequest, EntityManager);
		return;
	}
	
	const FMassActorSpawnRequest& MassActorSpawnRequest = SpawnRequest.Get<const FMassActorSpawnRequest>();
	const FMassEntityHandle EntityHandle = MassActorSpawnRequest.MassAgent;
	
	//FMassActorFragment* ActorFragment = EntityManager->GetFragmentDataPtr<FMassActorFragment>(EntityHandle);
	//AActor* ExistingActor = ActorFragment->Get();
	
		
	for (TWeakObjectPtr<AMassNPCCharacter> MassNPCCharacter : EventSystem->ClientActiveRealActors)
	{
		if (MassNPCCharacter.IsValid() == true && 
			MassNPCCharacter->MassState == EMassState::WaitingForMassActor &&
			MassNPCCharacter->MassEntityHandle == EntityHandle)
		{
			FTransformFragment* TransformFragment = EntityManager->GetFragmentDataPtr<FTransformFragment>(EntityHandle);
			TransformFragment->GetMutableTransform() = MassNPCCharacter->GetActorTransform();
			MassNPCCharacter->HideSelf();
			break;
		}
	}
	
	FMassActorFragment* ActorFragment = EntityManager->GetFragmentDataPtr<FMassActorFragment>(EntityHandle);
	if (ActorFragment->GetMutable() != nullptr && ActorFragment->IsOwnedByMass() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PreSpawn Before Reset] Entity=(%d,%d) Actor=%s OwnedByMass=%d"), EntityHandle.Index, EntityHandle.SerialNumber, 
				*GetNameSafe(ActorFragment->Get()), ActorFragment->IsOwnedByMass() ? 1 : 0);
		
		ActorFragment->ResetAndUpdateHandleMap();
	}
	
	Super::OnPreActorSpawn(SpawnRequestHandle, SpawnRequest, EntityManager);
}