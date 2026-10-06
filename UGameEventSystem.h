// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "NativeGameplayTags.h"
#include "MassNPC/ABullet.h"
#include "UGameEventSystem.generated.h"

class AMassNPCCharacter;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Mass_StateTree_Restart);

enum class EGameEventType : uint8
{
	None,
	GunShoot
};

struct FBeAttackEvent
{
	FMassEntityHandle VictimEntity;
	FVector Location = FVector::ZeroVector;
	float EffectRadius = 0.f;
};

struct FGameEvent
{
	EGameEventType Type = EGameEventType::GunShoot;

	FBeAttackEvent GunShootEvent;
};

UCLASS()
class OPENWORLD_API UUGameEventSystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UUGameEventSystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void PostInitialize();
	
	
	TArray<FGameEvent>& GetReadBuffer(){ return EventBuffers[ReadBufferIndex]; } 
	TArray<FGameEvent>& GetWriteBuffer(){ return EventBuffers[WriteBufferIndex]; }
	//TArray<TWeakObjectPtr<AMassNPCCharacter>>& GetActiveRealActors(){ return ServerActiveRealActors; }
	
	void OnMassPhaseFinished(float DeltaSeconds);
	void AddGameEvent(const FGameEvent& Event);
	void AddActorUpgradeRequest(const FMassEntityHandle EntityHandle);
	
private:
	void SwitchBuffers();
	
public:
	TArray<TWeakObjectPtr<AMassNPCCharacter>> ServerActiveRealActors;
	TArray<TWeakObjectPtr<AMassNPCCharacter>> ClientActiveRealActors;
	TArray<TWeakObjectPtr<AABullet>> ServerBullets;
	
private:
	UPROPERTY()
	TSubclassOf<AMassNPCCharacter> MassNPCCharacterClass;
	
	TArray<FGameEvent> EventBuffers[2];
	int32 WriteBufferIndex = 0;
	int32 ReadBufferIndex = 1;
	
	FDelegateHandle PhaseFinishedHandle;
	
	FCriticalSection ActorUpgradeRequestLock;
	TArray<FMassEntityHandle> PendingActorUpgradeRequests;
};
