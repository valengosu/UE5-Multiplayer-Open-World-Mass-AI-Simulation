#include "ClientMassDebugProcessor.h"
#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "Components/CapsuleComponent.h"
#include "MassExecutionContext.h"
#include "MassLODFragments.h"
#include "Translators/MassCapsuleComponentTranslators.h"

UClientMassDebugProcessor::UClientMassDebugProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    bRequiresGameThreadExecution = true;
    ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
}

void UClientMassDebugProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.Initialize(EntityManager);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadOnly);

    EntityQuery.AddRequirement<FCapsuleComponentWrapperFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
    EntityQuery.AddTagRequirement<FMassCapsuleTransformCopyToActorTag>(EMassFragmentPresence::Optional);
    EntityQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::Optional);
    EntityQuery.AddTagRequirement<FMassCapsuleTransformCopyToMassTag>(EMassFragmentPresence::Optional);
    //EntityQuery.RegisterWithProcessor(*this);
}

void UClientMassDebugProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ChunkContext)
    {
        const TConstArrayView<FTransformFragment> TransformFragments = ChunkContext.GetFragmentView<FTransformFragment>();
        const TConstArrayView<FMassActorFragment> ActorFragments = ChunkContext.GetFragmentView<FMassActorFragment>();
        const TConstArrayView<FCapsuleComponentWrapperFragment> CapsuleFragments = ChunkContext.GetFragmentView<FCapsuleComponentWrapperFragment>();
        const bool bCopyToMass = ChunkContext.DoesArchetypeHaveTag<FMassCapsuleTransformCopyToMassTag>();
        const bool bCopyToActor = ChunkContext.DoesArchetypeHaveTag<FMassCapsuleTransformCopyToActorTag>();

        const bool bHasCapsuleSyncTag = ChunkContext.DoesArchetypeHaveTag<FMassCapsuleTransformCopyToActorTag>();
        const bool bOffLOD = ChunkContext.DoesArchetypeHaveTag<FMassOffLODTag>();

        for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
        {
            const FMassEntityHandle EntityHandle = ChunkContext.GetEntity(i);
            const FVector EntityLocation = TransformFragments[i].GetTransform().GetLocation();

            const TCHAR* Side = GetWorld()->GetNetMode() == NM_Client ? TEXT("CLIENT") : TEXT("SERVER");
            const AActor* Actor = ActorFragments[i].Get();

            if (Actor)
            {
                UE_LOG(LogTemp, Warning, TEXT("[%s MASS] Entity[%d:%d] EntityLoc=%s ActorLoc=%s"), 
                    Side, EntityHandle.Index, EntityHandle.SerialNumber, *EntityLocation.ToString(), *Actor->GetActorLocation().ToString());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[%s MASS] Entity[%d:%d] EntityLoc=%s Actor=NULL"), 
                    Side, EntityHandle.Index, EntityHandle.SerialNumber, *EntityLocation.ToString());
            }

            const bool bHasCapsuleWrapper = !CapsuleFragments.IsEmpty();
            const UCapsuleComponent* Capsule = bHasCapsuleWrapper ? CapsuleFragments[i].Component.Get() : nullptr;

            UE_LOG(LogTemp, Warning, TEXT("[%s SYNC CHECK] Entity[%d:%d] Tag=%d Wrapper=%d Capsule=%s OffLOD=%d"), 
                Side, EntityHandle.Index, EntityHandle.SerialNumber, bHasCapsuleSyncTag ? 1 : 0, bHasCapsuleWrapper ? 1 : 0, Capsule ? *Capsule->GetName() : TEXT("NULL"), bOffLOD ? 1 : 0);
            
            UE_LOG(LogTemp, Warning, TEXT("[%s SYNC CHECK] Entity[%d:%d] ToMass=%d ToActor=%d OffLOD=%d"), 
                Side, EntityHandle.Index, EntityHandle.SerialNumber, bCopyToMass ? 1 : 0, bCopyToActor ? 1 : 0, bOffLOD ? 1 : 0);
            
            if (Capsule)
            {
                UE_LOG(LogTemp, Warning, TEXT("[%s CAPSULE] Entity[%d:%d] CapsuleLoc=%s"), Side, EntityHandle.Index, EntityHandle.SerialNumber, *Capsule->GetComponentLocation().ToString());
            }
        }
    });
}