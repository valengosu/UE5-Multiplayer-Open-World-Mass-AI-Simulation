#include "UGameEventSystem.h"
#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassMovementFragments.h"
#include "MassReplicationSubsystem.h"
#include "MassRepresentationActorManagement.h"
#include "MassSignalSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassStateTreeExecutionContext.h"
#include "MassStateTreeFragments.h"
#include "MassStateTreeProcessors.h"
#include "MassZoneGraphNavigationFragments.h"
#include "ZoneGraphSubsystem.h"
#include "./MassNPC/MassNPCCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MassNPC/Replication/UMyCrowdBubble.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Mass_StateTree_Restart, "Mass.StateTree.Restart");

UUGameEventSystem::UUGameEventSystem()
{
	static ConstructorHelpers::FClassFinder<AMassNPCCharacter> NPCClass(TEXT("/Game/NPC/MassNPCCharacter/BP_AMassNPCCharacter"));
	if (NPCClass.Succeeded())
	{
		MassNPCCharacterClass = NPCClass.Class;
	}
}

void UUGameEventSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UMassSimulationSubsystem>();
	Super::Initialize(Collection);
	
	if (GetWorld()->GetNetMode() == NM_ListenServer || GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		UMassSimulationSubsystem* MassSimulation = GetWorld()->GetSubsystem<UMassSimulationSubsystem>();
		PhaseFinishedHandle = MassSimulation->GetOnProcessingPhaseFinished(EMassProcessingPhase::PrePhysics).AddUObject(this, &UUGameEventSystem::OnMassPhaseFinished);	
	}
}

void UUGameEventSystem::Deinitialize()
{
	if (GetWorld()->GetNetMode() == NM_ListenServer || GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		UMassSimulationSubsystem* MassSimulation = GetWorld()->GetSubsystem<UMassSimulationSubsystem>();
		MassSimulation->GetOnProcessingPhaseFinished(EMassProcessingPhase::PrePhysics).Remove(PhaseFinishedHandle);
	}
	
	Super::Deinitialize();
}

void UUGameEventSystem::PostInitialize()
{
	Super::PostInitialize();
	
	UMassReplicationSubsystem* ReplicationSubsystem = GetWorld()->GetSubsystem<UMassReplicationSubsystem>();
	check(ReplicationSubsystem);

	ReplicationSubsystem->RegisterBubbleInfoClass(AMyCrowdClientBubbleInfo::StaticClass());
}

void UUGameEventSystem::OnMassPhaseFinished(float DeltaSeconds)
{
	check(IsInGameThread());
	
	SwitchBuffers();

	FMassEntityManager& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	
	for (auto EntityHandle : PendingActorUpgradeRequests)
	{
		if (EntityManager.IsEntityValid(EntityHandle) == false)
			continue;
		
		//Remove StateTree for future Restart!
		EntityManager.Defer().RemoveFragment<FMassStateTreeInstanceFragment>(EntityHandle);
		EntityManager.Defer().RemoveTag<FMassStateTreeActivatedTag>(EntityHandle);
		
		const FTransformFragment& TransformFragment = EntityManager.GetFragmentDataChecked<FTransformFragment>(EntityHandle);
		const FTransform SpawnTransform = TransformFragment.GetTransform();
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		AMassNPCCharacter* RealActor = GetWorld()->SpawnActor<AMassNPCCharacter>(MassNPCCharacterClass, SpawnTransform, SpawnParams);
		ServerActiveRealActors.Add(RealActor);
		
		RealActor->MassEntityHandle = EntityHandle;
		UMassReplicationSubsystem* ReplicationSubsystem = GetWorld()->GetSubsystem<UMassReplicationSubsystem>();
		RealActor->MassNetworkID = ReplicationSubsystem->GetNetIDFromHandle(EntityHandle);
		
		const FReactionFragment& ReactionFragment = EntityManager.GetFragmentDataChecked<FReactionFragment>(EntityHandle);
		RealActor->StartReact(ReactionFragment);
		
		FMassActorFragment* ActorFragment = EntityManager.GetFragmentDataPtr<FMassActorFragment>(EntityHandle);
		if (ActorFragment->Get() != nullptr)
		{
			UMassRepresentationActorManagement::ReleaseAnyActorOrCancelAnySpawning(EntityManager, EntityHandle, true);	
		}
		
		ActorFragment = EntityManager.GetFragmentDataPtr<FMassActorFragment>(EntityHandle);
		ActorFragment->SetAndUpdateHandleMap(EntityHandle, RealActor, false);
	}
	
	PendingActorUpgradeRequests.Reset();
	
	for (int32 i = ServerActiveRealActors.Num() - 1; i >= 0; --i)
	{
		if (ServerActiveRealActors[i].IsValid() == false)
		{
			ServerActiveRealActors.RemoveAtSwap(i);
			continue;
		}
		
		AMassNPCCharacter* RealActor = ServerActiveRealActors[i].Get();
		
		if (RealActor->MassState != EMassState::ReadyToHandBack)
			continue;
		
		if (EntityManager.IsEntityValid(RealActor->MassEntityHandle) == false)
		{
			UE_LOG(LogTemp, Error, TEXT("RealActor lost its Mass Entity! Actor=%s, Entity Index=%d, Serial=%d"), *GetNameSafe(RealActor), RealActor->MassEntityHandle.Index, RealActor->MassEntityHandle.SerialNumber);			ServerActiveRealActors.RemoveAtSwap(i);
			continue;
		}
		
		const FZoneGraphLaneLocation& ReturnLocation = RealActor->ReturnToMassLaneLocation;
		
		FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(RealActor->MassEntityHandle);
		check(TransformFragment);
		
		FTransform NewTransform = RealActor->GetActorTransform();
		NewTransform.SetLocation(ReturnLocation.Position);
		TransformFragment->GetMutableTransform() = NewTransform;
		
		UZoneGraphSubsystem* ZoneGraphSubsystem = GetWorld()->GetSubsystem<UZoneGraphSubsystem>();
		FMassZoneGraphLaneLocationFragment* LaneLocationFragment = EntityManager.GetFragmentDataPtr<FMassZoneGraphLaneLocationFragment>(RealActor->MassEntityHandle);
		
		float LaneLength = 0.0f;
		ZoneGraphSubsystem->GetLaneLength(ReturnLocation.LaneHandle, LaneLength);
		LaneLocationFragment->LaneHandle = ReturnLocation.LaneHandle;
		LaneLocationFragment->DistanceAlongLane = ReturnLocation.DistanceAlongLane;
		LaneLocationFragment->LaneLength = LaneLength;
		
		if (FMassZoneGraphCachedLaneFragment* CachedLane = EntityManager.GetFragmentDataPtr<FMassZoneGraphCachedLaneFragment>(RealActor->MassEntityHandle))
		{
			CachedLane->Reset();
		}

		if (FMassZoneGraphShortPathFragment* ShortPath = EntityManager.GetFragmentDataPtr<FMassZoneGraphShortPathFragment>(RealActor->MassEntityHandle))
		{
			ShortPath->Reset();
		}
		
		//Add StateTree for Restart!
		EntityManager.Defer().AddFragment<FMassStateTreeInstanceFragment>(RealActor->MassEntityHandle);
		
		FMassActorFragment* ActorFragment = EntityManager.GetFragmentDataPtr<FMassActorFragment>(RealActor->MassEntityHandle);
		ActorFragment->ResetAndUpdateHandleMap();
		
		RealActor->MassState = EMassState::WaitingForMassActor;
		
		// Hidden replicates to clients and causes a visual gap before Representation takes over, so do not use it!!
		//RealActor->SetActorHiddenInGame(true);
		RealActor->SetActorEnableCollision(false);
		RealActor->SetActorTickEnabled(false);
		RealActor->GetCharacterMovement()->DisableMovement();
		
		RealActor->SetLifeSpan(5.0f);
	}
}

void UUGameEventSystem::AddGameEvent(const FGameEvent& Event)
{
	check(IsInGameThread());
	GetWriteBuffer().Add(Event);
}

void UUGameEventSystem::AddActorUpgradeRequest(const FMassEntityHandle EntityHandle)
{
	FScopeLock Lock(&ActorUpgradeRequestLock);
	PendingActorUpgradeRequests.AddUnique(EntityHandle);
	UE_LOG(LogTemp, Warning, TEXT("Upgrade Entity: Index=%d Serial=%d"), EntityHandle.Index, EntityHandle.SerialNumber);
}

void UUGameEventSystem::SwitchBuffers()
{
	Swap(WriteBufferIndex, ReadBufferIndex);
	EventBuffers[WriteBufferIndex].Reset();
}
