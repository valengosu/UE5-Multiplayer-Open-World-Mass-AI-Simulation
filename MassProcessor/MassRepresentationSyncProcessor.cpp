#include "MassRepresentationSyncProcessor.h"

#include "MassCommonFragments.h"
#include "MassActorSubsystem.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MassNPC/MassNPC.h"

UMassRepresentationSyncProcessor::UMassRepresentationSyncProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    bRequiresGameThreadExecution = true;

    ExecutionFlags = (int32)EProcessorExecutionFlags::Client;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UMassRepresentationSyncProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.Initialize(EntityManager);

    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadOnly);

    EntityQuery.RegisterWithProcessor(*this);
}

void UMassRepresentationSyncProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ChunkContext)
    {
        TConstArrayView<FTransformFragment> TransformFragments = ChunkContext.GetFragmentView<FTransformFragment>();
        TConstArrayView<FMassActorFragment> ActorFragments = ChunkContext.GetFragmentView<FMassActorFragment>();

        for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
        {
            const AMassNPC* Character = Cast<AMassNPC>(ActorFragments[i].Get());
            
            if (Character == nullptr)
                continue;

            UCapsuleComponent* Capsule = Character->GetCapsuleComponent();

            FVector TargetLocation = TransformFragments[i].GetTransform().GetLocation();
            TargetLocation.Z += Capsule->GetScaledCapsuleHalfHeight();

            const FVector CurrentLocation = Capsule->GetComponentLocation();
            FVector ToTarget = TargetLocation - CurrentLocation;
            const float MoveDistance = Character->GetCharacterMovement()->Velocity.Size() * Context.GetDeltaTimeSeconds();
            //UE_LOG(LogTemp, Warning, TEXT("MoveDistance = %f"), MoveDistance);
            const FVector NewLocation = CurrentLocation + ToTarget.GetSafeNormal() * FMath::Min(MoveDistance, ToTarget.Size());
            //UE_LOG(LogTemp, Warning, TEXT("NewLocation = %s"), *NewLocation.ToString());
            FQuat NewRotation = Capsule->GetComponentQuat();
            
            ToTarget.Z = 0.f;
            if (ToTarget.IsNearlyZero() == false)
            {
                //const FQuat TargetRotation = ToTarget.ToOrientationQuat();
                FVector Velocity = Character->GetCharacterMovement()->Velocity;
                Velocity.Z = 0.0f;
                NewRotation = FMath::QInterpTo(NewRotation, Velocity.ToOrientationQuat(), Context.GetDeltaTimeSeconds(), 8.0f);
            }
            
            Capsule->SetWorldLocationAndRotation(NewLocation, NewRotation, false);
        }
    });
}