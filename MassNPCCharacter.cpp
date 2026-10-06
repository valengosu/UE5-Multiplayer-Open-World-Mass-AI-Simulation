#include "MassNPCCharacter.h"
#include "AIController.h"
#include "MassActorSpawnerSubsystem.h"
#include "MassActorSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MassCommonFragments.h"
#include "Net/UnrealNetwork.h"
#include "MassEntitySubsystem.h"
#include "MassNPC.h"
#include "MassReplicationSubsystem.h"
#include "MassRepresentationFragments.h"
#include "MyMassCrowdRepresentationSubsystem.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimNode_StateMachine.h"
#include "AnimNodes/AnimNode_BlendSpacePlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

AMassNPCCharacter::AMassNPCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void AMassNPCCharacter::PostNetInit()
{
	Super::PostNetInit();
	
	UMassReplicationSubsystem* ReplicationSubsystem = GetWorld()->GetSubsystem<UMassReplicationSubsystem>();
	FMassEntityHandle ClientEntityHandle = ReplicationSubsystem->FindEntity(MassNetworkID);
	this->MassEntityHandle = ClientEntityHandle;
	
	FMassEntityManager& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	if (!EntityManager.IsEntityValid(ClientEntityHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientEntity not ready!!!"));
		Destroy();
		return;
	}
	
	FMassActorFragment* ActorFragment = EntityManager.GetFragmentDataPtr<FMassActorFragment>(ClientEntityHandle);

	if (const AMassNPC* RepresentationActor = Cast<AMassNPC>(ActorFragment->Get()))
	{
		//UE_LOG(LogTemp, Warning, TEXT("PostNetInit Before: %s, Location: %s"), *GetName(), *GetActorLocation().ToString());
		
		SetActorTransform(ActorFragment->Get()->GetActorTransform());
		GetCharacterMovement()->Velocity = RepresentationActor->GetCharacterMovement()->Velocity;
		
		if (CopyAnimationState(RepresentationActor->GetMesh(), AnimHandoffState) == true)
		{
			UAnimInstance* DestAnim = this->GetMesh()->GetAnimInstance();
			FDoubleProperty* SpeedProperty = FindFProperty<FDoubleProperty>(DestAnim->GetClass(), TEXT("Speed"));
			FDoubleProperty* MoveRotationProperty = FindFProperty<FDoubleProperty>(DestAnim->GetClass(), TEXT("MoveRotation"));
			check(SpeedProperty && MoveRotationProperty);
			SpeedProperty->SetPropertyValue_InContainer(DestAnim, AnimHandoffState.Speed);
			MoveRotationProperty->SetPropertyValue_InContainer(DestAnim, AnimHandoffState.MoveRotation);
		}
		
		UMyMassCrowdRepresentationSubsystem* MyRepresentationSubsystem = Cast<UMyMassCrowdRepresentationSubsystem>(EntityManager.GetSharedFragmentDataChecked<FMassRepresentationSubsystemSharedFragment>(ClientEntityHandle).RepresentationSubsystem);
		MyRepresentationSubsystem->ReleaseMassRepresentationForRealActorTakeover(EntityManager, ClientEntityHandle, true);
		
		//UE_LOG(LogTemp, Warning, TEXT("PostNetInit After: %s, Location: %s"), *GetName(), *GetActorLocation().ToString());
	}
	
	else if (AMassNPCCharacter* OldRealActor = Cast<AMassNPCCharacter>(ActorFragment->GetMutable()))
	{
		OldRealActor->HideSelf();
		OldRealActor->MassEntityHandle = FMassEntityHandle();
		OldRealActor->MassNetworkID = FMassNetworkID();
		
		ActorFragment->ResetAndUpdateHandleMap();
	}
	
	ActorFragment = EntityManager.GetFragmentDataPtr<FMassActorFragment>(ClientEntityHandle);
	ActorFragment->SetAndUpdateHandleMap(ClientEntityHandle, this, false);
	
	UUGameEventSystem* GameEventSystem = GetWorld()->GetSubsystem<UUGameEventSystem>();
	GameEventSystem->ClientActiveRealActors.Add(this);
}

void AMassNPCCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMassNPCCharacter, MassNetworkID);
	DOREPLIFETIME(AMassNPCCharacter, MassState);
}

void AMassNPCCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UUGameEventSystem* GameEventSystem = GetWorld()->GetSubsystem<UUGameEventSystem>();
	if (GameEventSystem != nullptr)
	{
		if (HasAuthority())
		{
			GameEventSystem->ServerActiveRealActors.Remove(this);
		}
		else
		{
			GameEventSystem->ClientActiveRealActors.Remove(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

// Called when the game starts or when spawned
void AMassNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	MassState = EMassState::Active;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	
	//UE_LOG(LogTemp, Warning, TEXT("[Client Mass -> RealActor] ActorLocation=%s"), *GetActorLocation().ToString());
}

void AMassNPCCharacter::OnRep_MassState()
{
	if (MassState == EMassState::WaitingForMassActor)
	{
		if (MassNetworkID.IsValid() == false || MassEntityHandle.IsValid() == false)
			return;
		
		UMassReplicationSubsystem* ReplicationSubsystem = GetWorld()->GetSubsystem<UMassReplicationSubsystem>();
		FMassEntityHandle ClientEntityHandle = ReplicationSubsystem->FindEntity(MassNetworkID);
	
		FMassEntityManager& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
		if (!EntityManager.IsEntityValid(ClientEntityHandle))
			return;
		
		FMassActorFragment* ActorFragment = EntityManager.GetFragmentDataPtr<FMassActorFragment>(ClientEntityHandle);
		if (ActorFragment->Get() != this)
		{
			UE_LOG(LogTemp, Warning, TEXT("Why!!!!!ClientEntityHandle: Index = %d, Serial = %d"), ClientEntityHandle.Index, ClientEntityHandle.SerialNumber);
			return;
		}
		
		ActorFragment->ResetAndUpdateHandleMap();
		
		FTransform ActorTransform = GetActorTransform();
		FVector Location = ActorTransform.GetLocation();
		Location.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		
		ActorTransform.SetLocation(Location);
		
		FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(ClientEntityHandle);
		TransformFragment->SetTransform(ActorTransform);
	}
}

void AMassNPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("[%s] Tick: %s | This=%p | Location=%s"),HasAuthority() ? TEXT("Server") : TEXT("Client"),*GetName(),this,*GetActorLocation().ToString());
}

void AMassNPCCharacter::StartReact(const FReactionFragment& ReactionFragment)
{
	if (ReactionFragment.EventType == EGameEventType::GunShoot)
	{
		AAIController* AIController = Cast<AAIController>(GetController());
		UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
		Blackboard->SetValueAsVector(TEXT("EventLocation"), ReactionFragment.EventLocation);
		UE_LOG(LogTemp, Warning, TEXT("EventLocation: %s"), *ReactionFragment.EventLocation.ToString());
		
		if (ReactionFragment.SourceCharacterHandle == this->MassEntityHandle)
		{
			Blackboard->SetValueAsBool(TEXT("IsHit"), true);
		}
		else
		{
			Blackboard->SetValueAsBool(TEXT("IsObserving"), true);
		}
	}
}

void AMassNPCCharacter::HideSelf()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void AMassNPCCharacter::PlayHitAnimation_Implementation()
{
	if (HitAnimation == nullptr)
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr)
		return;

	AnimInstance->PlaySlotAnimationAsDynamicMontage(HitAnimation, TEXT("DefaultSlot"));
}

void AMassNPCCharacter::PostNetReceiveLocationAndRotation()
{
	const FRepMovement& ServerMovement = GetReplicatedMovement();
	
	FVector ServerLocation = FRepMovement::RebaseOntoLocalOrigin(ServerMovement.Location, this);
	float Offset = FVector::Dist2D(ServerLocation, GetActorLocation());
	
	if (Offset > 45.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Offset=%.3f"), Offset);
	}
	
	if (Offset < 45.f && ServerMovement.LinearVelocity.Length() <= 5.f)
	{
		SetActorRotation(ServerMovement.Rotation);
		return;
	}
		
	Super::PostNetReceiveLocationAndRotation();
}

bool AMassNPCCharacter::CopyAnimationState(USkeletalMeshComponent* SourceMesh, FAnimHandoffState &HandoffState)
{
	UAnimInstance* SourceAnim = SourceMesh->GetAnimInstance();
	if (SourceAnim == nullptr)
		return false;
	
	FDoubleProperty* SpeedProperty = FindFProperty<FDoubleProperty>(SourceAnim->GetClass(),TEXT("Speed"));
	FDoubleProperty* MoveRotationProperty = FindFProperty<FDoubleProperty>(SourceAnim->GetClass(),TEXT("MoveRotation"));
	if (SpeedProperty == nullptr || MoveRotationProperty == nullptr)
		return false;

	HandoffState.Speed = SpeedProperty->GetPropertyValue_InContainer(SourceAnim);
	HandoffState.MoveRotation = MoveRotationProperty->GetPropertyValue_InContainer(SourceAnim);

	const int32 MachineIndex = SourceAnim->GetStateMachineIndex(TEXT("Locomotion"));
	const FAnimNode_StateMachine* StateMachine = SourceAnim->GetStateMachineInstance(MachineIndex);
	const int32 StateIndex = StateMachine->GetCurrentState();

	HandoffState.StateName = StateMachine->GetCurrentStateName();
	HandoffState.TimeFraction = SourceAnim->GetRelevantAnimTimeFraction(MachineIndex, StateIndex);
	
	return true;
}

