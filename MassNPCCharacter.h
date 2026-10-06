// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassAgentComponent.h"
#include "ZoneGraphTypes.h"
#include "GameFramework/Character.h"
#include "FReactionFragment.h"
#include "MassNPCCharacter.generated.h"

class AMassNPC;

UENUM()
enum class EMassState : uint8
{
	Active,
	ReadyToHandBack,
	WaitingForMassActor
};

USTRUCT(BlueprintType)
struct FAnimHandoffState
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	double Speed;
	
	UPROPERTY(BlueprintReadWrite)
	double MoveRotation;
	
	UPROPERTY(BlueprintReadWrite)
	FName StateName;
	
	UPROPERTY(BlueprintReadWrite)
	float TimeFraction;
};

UCLASS()
class OPENWORLD_API AMassNPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMassNPCCharacter();
	virtual void PostNetInit() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostNetReceiveLocationAndRotation() override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRep_MassState();
	bool CopyAnimationState(USkeletalMeshComponent* SourceMesh, FAnimHandoffState &AnimHandoffState);

public:
	virtual void Tick(float DeltaTime) override;
	void StartReact(const FReactionFragment& ReactionFragment);
		
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void PlayHitAnimation();
	
	UFUNCTION(BlueprintPure)
	float GetHitAnimationLength() const
	{
		return HitAnimation ? HitAnimation->GetPlayLength() : 0.f;
	}

	void HideSelf();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	TObjectPtr<UAnimSequence> HitAnimation;
	
	UPROPERTY(Replicated)
	FMassNetworkID MassNetworkID;
	
	UPROPERTY(ReplicatedUsing=OnRep_MassState)
	EMassState MassState;
	
	FMassEntityHandle MassEntityHandle;
	FZoneGraphLaneLocation ReturnToMassLaneLocation;
	
	UPROPERTY(BlueprintReadOnly)
	FAnimHandoffState AnimHandoffState;
};
