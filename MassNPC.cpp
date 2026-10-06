// Fill out your copyright notice in the Description page of Project Settings.
#include "MassNPC.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AMassNPC::AMassNPC()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->DisableMovement();
}

void AMassNPC::BeginPlay()
{
	Super::BeginPlay();
	//GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Walking;
	PreviousMassUpdateTime = GetWorld()->GetTimeSeconds();
}

void AMassNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	GetCharacterMovement()->Velocity = FMath::VInterpTo(GetCharacterMovement()->Velocity, TargetMassVelocity, DeltaTime, 5.0f);
}
